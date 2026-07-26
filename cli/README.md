## Tools
```bash
go install google.golang.org/protobuf/cmd/protoc-gen-go@latest
```

```bash
protoc   -I=../../protobuf   --go_out=generated   --go_opt=paths=source_relative   --go_opt=Mdebugger.proto=drunkpc-debugger/generated   --go-grpc_out=generated   --go-grpc_opt=paths=source_relative   --go-grpc_opt=Mdebugger.proto=drunkpc-debugger/generated   debugger.proto
```
