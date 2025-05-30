// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: extractor.proto

#include "extractor.pb.h"
#include "extractor.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/method_handler_impl.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>
namespace extractor {

static const char* ExtractorService_method_names[] = {
  "/extractor.ExtractorService/GetAllData",
};

std::unique_ptr< ExtractorService::Stub> ExtractorService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< ExtractorService::Stub> stub(new ExtractorService::Stub(channel));
  return stub;
}

ExtractorService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_GetAllData_(ExtractorService_method_names[0], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status ExtractorService::Stub::GetAllData(::grpc::ClientContext* context, const ::extractor::AllDataSend& request, ::extractor::AllDataResponse* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_GetAllData_, context, request, response);
}

void ExtractorService::Stub::experimental_async::GetAllData(::grpc::ClientContext* context, const ::extractor::AllDataSend* request, ::extractor::AllDataResponse* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_GetAllData_, context, request, response, std::move(f));
}

::grpc::ClientAsyncResponseReader< ::extractor::AllDataResponse>* ExtractorService::Stub::AsyncGetAllDataRaw(::grpc::ClientContext* context, const ::extractor::AllDataSend& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::extractor::AllDataResponse>::Create(channel_.get(), cq, rpcmethod_GetAllData_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::extractor::AllDataResponse>* ExtractorService::Stub::PrepareAsyncGetAllDataRaw(::grpc::ClientContext* context, const ::extractor::AllDataSend& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::extractor::AllDataResponse>::Create(channel_.get(), cq, rpcmethod_GetAllData_, context, request, false);
}

ExtractorService::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      ExtractorService_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< ExtractorService::Service, ::extractor::AllDataSend, ::extractor::AllDataResponse>(
          std::mem_fn(&ExtractorService::Service::GetAllData), this)));
}

ExtractorService::Service::~Service() {
}

::grpc::Status ExtractorService::Service::GetAllData(::grpc::ServerContext* context, const ::extractor::AllDataSend* request, ::extractor::AllDataResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace extractor

