package main

import "os"
import "fmt"
import "log"
import "google.golang.org/protobuf/proto"
import pb "DrunkPCDebugger/generated"
import "DrunkPCDebugger/internal/transport"
import "DrunkPCDebugger/internal/args"
import "DrunkPCDebugger/internal/logging"

func main() {
	args := args.Parse()

	if args.OnlyShowVersion {
		os.Exit(1)
	}

	loggerCore, err := logging.NewAppLogger(args.LogLevel)
	if err != nil {
		log.Fatalf("failed to init logger: %v", err)
	}
	defer loggerCore.Sync()
	logger := loggerCore.Sugar()

	tr, err := transport.NewSerial(args.Port, 115200, logger)
	if err != nil {
		logger.Fatalf("%v", err)
	}
	fmt.Printf("%v %v\n", tr)

	request := &pb.RpcRequest{
		RequestId: 1,
		Payload: &pb.RpcRequest_AcquireBus{
			AcquireBus: &pb.AcquireBusRequest{},
		},
	}

	requestData, err := proto.Marshal(request)
	if err != nil {
		//return nil, fmt.Errorf("marshal request: %w", err)
	}
	fmt.Printf("%v\n", requestData)
}
