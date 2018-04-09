#include "message_handler.h"
#include "query_utils.h"
#include "queue_manager.h"

MAKE_REFLECT_STRUCT(QueryFile::Def,
                    file,
                    path,
                    args,
                    language,
                    outline,
                    all_symbols,
                    inactive_regions,
                    dependencies);

namespace {
MethodType kMethodType = "$ccls/fileInfo";

struct lsDocumentSymbolParams {
  lsTextDocumentIdentifier textDocument;
};
MAKE_REFLECT_STRUCT(lsDocumentSymbolParams, textDocument);

struct In_CclsFileInfo : public RequestInMessage {
  MethodType GetMethodType() const override { return kMethodType; }
  lsDocumentSymbolParams params;
};
MAKE_REFLECT_STRUCT(In_CclsFileInfo, id, params);
REGISTER_IN_MESSAGE(In_CclsFileInfo);

struct Out_CclsFileInfo : public lsOutMessage<Out_CclsFileInfo> {
  lsRequestId id;
  QueryFile::Def result;
};
MAKE_REFLECT_STRUCT(Out_CclsFileInfo, jsonrpc, id, result);

struct Handler_CclsFileInfo : BaseMessageHandler<In_CclsFileInfo> {
  MethodType GetMethodType() const override { return kMethodType; }
  void Run(In_CclsFileInfo* request) override {
    QueryFile* file;
    if (!FindFileOrFail(db, project, request->id,
                        request->params.textDocument.uri.GetPath(), &file)) {
      return;
    }

    Out_CclsFileInfo out;
    out.id = request->id;
    // Expose some fields of |QueryFile::Def|.
    out.result.path = file->def->path;
    out.result.args = file->def->args;
    out.result.language = file->def->language;
    out.result.includes = file->def->includes;
    out.result.inactive_regions = file->def->inactive_regions;
    QueueManager::WriteStdout(kMethodType, out);
  }
};
REGISTER_MESSAGE_HANDLER(Handler_CclsFileInfo);
}  // namespace
