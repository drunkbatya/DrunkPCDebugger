#include <rpc/rpc.h>

#include <bus/bus.h>
#include <bus/power.h>
#include <eeprom/at28c256.h>
#include <kal/usb_uart.h>

#include <debugger.pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

#define RPC_HEADER_SIZE      (2U)
#define RPC_MAX_PAYLOAD_SIZE (4096U)
#define RPC_RX_TIMEOUT_MS    (1000U)
#define RPC_TX_TIMEOUT_MS    (1000U)

static uint8_t rpc_frame[RPC_HEADER_SIZE + RPC_MAX_PAYLOAD_SIZE];
static debugger_RpcRequest rpc_request;
static debugger_RpcResponse rpc_response;

static debugger_ErrorResponse rpc_make_error(debugger_ErrType err_type) {
    debugger_ErrorResponse error = debugger_ErrorResponse_init_zero;
    error.err_type = err_type;
    return error;
}

static bool rpc_read_exact(uint8_t* data, size_t size) {
    return kal_usb_uart_rx(data, size, RPC_RX_TIMEOUT_MS) == size;
}

static uint16_t rpc_decode_header(const uint8_t* header) {
    return (uint16_t)header[0] | ((uint16_t)header[1] << 8);
}

static void rpc_encode_header(uint8_t* header, uint16_t size) {
    header[0] = (uint8_t)(size & 0xFFU);
    header[1] = (uint8_t)((size >> 8) & 0xFFU);
}

static bool rpc_read_frame(uint16_t* length) {
    uint8_t header[RPC_HEADER_SIZE];

    if(kal_usb_uart_get_rx_available() < RPC_HEADER_SIZE) return false;
    if(!rpc_read_exact(header, RPC_HEADER_SIZE)) return false;

    const uint16_t size = rpc_decode_header(header);
    if(size > RPC_MAX_PAYLOAD_SIZE || !rpc_read_exact(rpc_frame, size)) {
        kal_usb_uart_rx_flush();
        return false;
    }

    *length = size;
    return true;
}

static bool rpc_decode_request(uint16_t length) {
    rpc_request = (debugger_RpcRequest)debugger_RpcRequest_init_zero;

    pb_istream_t stream = pb_istream_from_buffer(rpc_frame, length);
    return pb_decode(&stream, debugger_RpcRequest_fields, &rpc_request);
}

static bool rpc_is_range_valid(uint32_t address, uint32_t size) {
    if(size > debugger_Const_CONST_MAX_CHUNK_SIZE) return false;
    if(address > BUS_ADDRESS_MAX) return false;
    return (address + size) <= BUS_ADDRESS_SPACE;
}

static void rpc_handle_bus_control(void) {
    const bool acquire = rpc_request.payload.bus_control_request.acquire;
    const bool succeeded = acquire ? bus_acquire() : bus_release();

    rpc_response.which_payload = debugger_RpcResponse_bus_control_request_tag;
    rpc_response.payload.bus_control_request = rpc_make_error(
        succeeded ? debugger_ErrType_ERROR_TYPE_OK : debugger_ErrType_ERROR_TYPE_TIMEOUT);
}

static void rpc_handle_power_on_bus(void) {
    bus_power_set_enabled(rpc_request.payload.power_on_bus_request.enable);

    rpc_response.which_payload = debugger_RpcResponse_power_on_bus_request_tag;
    rpc_response.payload.power_on_bus_request = rpc_make_error(debugger_ErrType_ERROR_TYPE_OK);
}

static debugger_ErrType rpc_write_flash(const debugger_WriteFlashRequest* request) {
    if(!bus_is_acquired()) return debugger_ErrType_ERROR_TYPE_BUS_NOT_ACQUIRED;
    if(!rpc_is_range_valid(request->address, request->data.size)) {
        return debugger_ErrType_ERROR_TYPE_OUT_OF_RANGE;
    }
    if(!at28c256_write((uint16_t)request->address, request->data.bytes, request->data.size)) {
        return debugger_ErrType_ERROR_TYPE_TIMEOUT;
    }
    return debugger_ErrType_ERROR_TYPE_OK;
}

static void rpc_handle_write_flash(void) {
    const debugger_ErrType err_type = rpc_write_flash(&rpc_request.payload.write_flash);

    rpc_response.which_payload = debugger_RpcResponse_write_flash_tag;
    rpc_response.payload.write_flash = rpc_make_error(err_type);
}

static debugger_ErrType
    rpc_read_flash(const debugger_ReadFlashRequest* request, debugger_ReadFlashResponse* response) {
    if(!bus_is_acquired()) return debugger_ErrType_ERROR_TYPE_BUS_NOT_ACQUIRED;
    if(!rpc_is_range_valid(request->address, request->size)) {
        return debugger_ErrType_ERROR_TYPE_OUT_OF_RANGE;
    }
    if(!at28c256_read((uint16_t)request->address, response->data.bytes, request->size)) {
        return debugger_ErrType_ERROR_TYPE_FATAL;
    }

    response->data.size = (pb_size_t)request->size;
    return debugger_ErrType_ERROR_TYPE_OK;
}

static void rpc_handle_read_flash(void) {
    rpc_response.which_payload = debugger_RpcResponse_read_flash_tag;

    debugger_ReadFlashResponse* response = &rpc_response.payload.read_flash;
    const debugger_ErrType err_type = rpc_read_flash(&rpc_request.payload.read_flash, response);

    response->has_status = true;
    response->status = rpc_make_error(err_type);
}

static bool rpc_report_rx_overrun(void) {
    if(!kal_usb_uart_take_rx_overrun()) return false;

    kal_usb_uart_rx_flush();
    rpc_response = (debugger_RpcResponse)debugger_RpcResponse_init_zero;
    rpc_response.which_payload = debugger_RpcResponse_generic_tag;
    rpc_response.payload.generic = rpc_make_error(debugger_ErrType_ERROR_TYPE_RX_OVERRUN);
    return true;
}

static void rpc_handle_bad_request(void) {
    rpc_response.which_payload = debugger_RpcResponse_generic_tag;
    rpc_response.payload.generic = rpc_make_error(debugger_ErrType_ERROR_TYPE_BAD_REQUEST);
}

static void rpc_handle_request(void) {
    rpc_response = (debugger_RpcResponse)debugger_RpcResponse_init_zero;
    rpc_response.request_id = rpc_request.request_id;

    switch(rpc_request.which_payload) {
    case debugger_RpcRequest_bus_control_request_tag:
        rpc_handle_bus_control();
        break;
    case debugger_RpcRequest_power_on_bus_request_tag:
        rpc_handle_power_on_bus();
        break;
    case debugger_RpcRequest_write_flash_tag:
        rpc_handle_write_flash();
        break;
    case debugger_RpcRequest_read_flash_tag:
        rpc_handle_read_flash();
        break;
    default:
        rpc_handle_bad_request();
        break;
    }
}

static bool rpc_send_response(void) {
    pb_ostream_t stream =
        pb_ostream_from_buffer(rpc_frame + RPC_HEADER_SIZE, RPC_MAX_PAYLOAD_SIZE);

    if(!pb_encode(&stream, debugger_RpcResponse_fields, &rpc_response)) return false;

    rpc_encode_header(rpc_frame, (uint16_t)stream.bytes_written);
    return kal_usb_uart_tx(rpc_frame, RPC_HEADER_SIZE + stream.bytes_written, RPC_TX_TIMEOUT_MS);
}

void rpc_init(void) {
    rpc_request = (debugger_RpcRequest)debugger_RpcRequest_init_zero;
    rpc_response = (debugger_RpcResponse)debugger_RpcResponse_init_zero;
}

void rpc_process(void) {
    uint16_t length = 0;

    if(rpc_report_rx_overrun()) {
        rpc_send_response();
        return;
    }

    if(!rpc_read_frame(&length)) return;

    if(rpc_decode_request(length)) {
        rpc_handle_request();
    } else {
        rpc_response = (debugger_RpcResponse)debugger_RpcResponse_init_zero;
        rpc_handle_bad_request();
    }

    rpc_send_response();
}
