package main

import (
	"os"
	"fmt"
	"log"
	"google.golang.org/protobuf/proto"
	pb "DrunkPCDebugger/generated"
	"DrunkPCDebugger/internal/transport"
	"DrunkPCDebugger/internal/args"
	"DrunkPCDebugger/internal/logging"
)

func main() {
	args := args.Parse()

	loggerCore, err := logging.NewAppLogger(args.LogLevel)
	if err != nil {
		log.Fatalf("failed to init logger: %v", err)
	}
	defer loggerCore.Sync()
	logger := loggerCore.Sugar()

	_, transportErr := transport.NewSerial(args.Port, 115200, logger)

	if args.OnlyShowVersion {
		cliVersion := "1.0.1"
		fmt.Printf("Tool:\n\t%s\n", cliVersion)

		if transportErr == nil {
			deviceVersion := "2.0.0"
			fmt.Printf("Device:\n\t%s\n", deviceVersion)
		}

		os.Exit(1)
	}

	if transportErr != nil {
		logger.Fatalf("%v", transportErr)
	}

	request := &pb.RpcRequest{
		RequestId: 1,
		Payload: &pb.RpcRequest_AcquireBus{
			AcquireBus: &pb.AcquireBusRequest{},
		},
	}

	requestData, err := proto.Marshal(request)
	if err != nil {
		fmt.Printf("%v\n", err)
	}
	fmt.Printf("%v\n", requestData)
}
