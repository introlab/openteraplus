// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: ServerCommand.proto

#include "ServerCommand.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG
namespace opentera {
namespace protobuf {
constexpr ServerCommand::ServerCommand(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized)
  : data_(nullptr)
  , type_(0)
{}
struct ServerCommandDefaultTypeInternal {
  constexpr ServerCommandDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~ServerCommandDefaultTypeInternal() {}
  union {
    ServerCommand _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT ServerCommandDefaultTypeInternal _ServerCommand_default_instance_;
}  // namespace protobuf
}  // namespace opentera
static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_ServerCommand_2eproto[1];
static const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* file_level_enum_descriptors_ServerCommand_2eproto[1];
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_ServerCommand_2eproto = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32 TableStruct_ServerCommand_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::ServerCommand, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::ServerCommand, type_),
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::ServerCommand, data_),
};
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, sizeof(::opentera::protobuf::ServerCommand)},
};

static ::PROTOBUF_NAMESPACE_ID::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::opentera::protobuf::_ServerCommand_default_instance_),
};

const char descriptor_table_protodef_ServerCommand_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\023ServerCommand.proto\022\021opentera.protobuf"
  "\032\031google/protobuf/any.proto\"\250\001\n\rServerCo"
  "mmand\022:\n\004type\030\001 \001(\0162,.opentera.protobuf."
  "ServerCommand.CommandType\022\"\n\004data\030\002 \001(\0132"
  "\024.google.protobuf.Any\"7\n\013CommandType\022\017\n\013"
  "CMD_UNKNOWN\020\000\022\027\n\023CMD_REGISTER_DEVICE\020\001b\006"
  "proto3"
  ;
static const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable*const descriptor_table_ServerCommand_2eproto_deps[1] = {
  &::descriptor_table_google_2fprotobuf_2fany_2eproto,
};
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_ServerCommand_2eproto_once;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_ServerCommand_2eproto = {
  false, false, 246, descriptor_table_protodef_ServerCommand_2eproto, "ServerCommand.proto", 
  &descriptor_table_ServerCommand_2eproto_once, descriptor_table_ServerCommand_2eproto_deps, 1, 1,
  schemas, file_default_instances, TableStruct_ServerCommand_2eproto::offsets,
  file_level_metadata_ServerCommand_2eproto, file_level_enum_descriptors_ServerCommand_2eproto, file_level_service_descriptors_ServerCommand_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable* descriptor_table_ServerCommand_2eproto_getter() {
  return &descriptor_table_ServerCommand_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY static ::PROTOBUF_NAMESPACE_ID::internal::AddDescriptorsRunner dynamic_init_dummy_ServerCommand_2eproto(&descriptor_table_ServerCommand_2eproto);
namespace opentera {
namespace protobuf {
const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* ServerCommand_CommandType_descriptor() {
  ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&descriptor_table_ServerCommand_2eproto);
  return file_level_enum_descriptors_ServerCommand_2eproto[0];
}
bool ServerCommand_CommandType_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
      return true;
    default:
      return false;
  }
}

#if (__cplusplus < 201703) && (!defined(_MSC_VER) || _MSC_VER >= 1900)
constexpr ServerCommand_CommandType ServerCommand::CMD_UNKNOWN;
constexpr ServerCommand_CommandType ServerCommand::CMD_REGISTER_DEVICE;
constexpr ServerCommand_CommandType ServerCommand::CommandType_MIN;
constexpr ServerCommand_CommandType ServerCommand::CommandType_MAX;
constexpr int ServerCommand::CommandType_ARRAYSIZE;
#endif  // (__cplusplus < 201703) && (!defined(_MSC_VER) || _MSC_VER >= 1900)

// ===================================================================

class ServerCommand::_Internal {
 public:
  static const PROTOBUF_NAMESPACE_ID::Any& data(const ServerCommand* msg);
};

const PROTOBUF_NAMESPACE_ID::Any&
ServerCommand::_Internal::data(const ServerCommand* msg) {
  return *msg->data_;
}
void ServerCommand::clear_data() {
  if (GetArenaForAllocation() == nullptr && data_ != nullptr) {
    delete data_;
  }
  data_ = nullptr;
}
ServerCommand::ServerCommand(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor();
  if (!is_message_owned) {
    RegisterArenaDtor(arena);
  }
  // @@protoc_insertion_point(arena_constructor:opentera.protobuf.ServerCommand)
}
ServerCommand::ServerCommand(const ServerCommand& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  if (from._internal_has_data()) {
    data_ = new PROTOBUF_NAMESPACE_ID::Any(*from.data_);
  } else {
    data_ = nullptr;
  }
  type_ = from.type_;
  // @@protoc_insertion_point(copy_constructor:opentera.protobuf.ServerCommand)
}

inline void ServerCommand::SharedCtor() {
::memset(reinterpret_cast<char*>(this) + static_cast<size_t>(
    reinterpret_cast<char*>(&data_) - reinterpret_cast<char*>(this)),
    0, static_cast<size_t>(reinterpret_cast<char*>(&type_) -
    reinterpret_cast<char*>(&data_)) + sizeof(type_));
}

ServerCommand::~ServerCommand() {
  // @@protoc_insertion_point(destructor:opentera.protobuf.ServerCommand)
  if (GetArenaForAllocation() != nullptr) return;
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

inline void ServerCommand::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  if (this != internal_default_instance()) delete data_;
}

void ServerCommand::ArenaDtor(void* object) {
  ServerCommand* _this = reinterpret_cast< ServerCommand* >(object);
  (void)_this;
}
void ServerCommand::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void ServerCommand::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void ServerCommand::Clear() {
// @@protoc_insertion_point(message_clear_start:opentera.protobuf.ServerCommand)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  if (GetArenaForAllocation() == nullptr && data_ != nullptr) {
    delete data_;
  }
  data_ = nullptr;
  type_ = 0;
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* ServerCommand::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // .opentera.protobuf.ServerCommand.CommandType type = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 8)) {
          ::PROTOBUF_NAMESPACE_ID::uint64 val = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
          _internal_set_type(static_cast<::opentera::protobuf::ServerCommand_CommandType>(val));
        } else goto handle_unusual;
        continue;
      // .google.protobuf.Any data = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 18)) {
          ptr = ctx->ParseMessage(_internal_mutable_data(), ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      default: {
      handle_unusual:
        if ((tag == 0) || ((tag & 7) == 4)) {
          CHK_(ptr);
          ctx->SetLastTag(tag);
          goto success;
        }
        ptr = UnknownFieldParse(tag,
            _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
            ptr, ctx);
        CHK_(ptr != nullptr);
        continue;
      }
    }  // switch
  }  // while
success:
  return ptr;
failure:
  ptr = nullptr;
  goto success;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* ServerCommand::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:opentera.protobuf.ServerCommand)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // .opentera.protobuf.ServerCommand.CommandType type = 1;
  if (this->_internal_type() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteEnumToArray(
      1, this->_internal_type(), target);
  }

  // .google.protobuf.Any data = 2;
  if (this->_internal_has_data()) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessage(
        2, _Internal::data(this), target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:opentera.protobuf.ServerCommand)
  return target;
}

size_t ServerCommand::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:opentera.protobuf.ServerCommand)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // .google.protobuf.Any data = 2;
  if (this->_internal_has_data()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(
        *data_);
  }

  // .opentera.protobuf.ServerCommand.CommandType type = 1;
  if (this->_internal_type() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::EnumSize(this->_internal_type());
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(
        _internal_metadata_, total_size, &_cached_size_);
  }
  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData ServerCommand::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    ServerCommand::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*ServerCommand::GetClassData() const { return &_class_data_; }

void ServerCommand::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message*to,
                      const ::PROTOBUF_NAMESPACE_ID::Message&from) {
  static_cast<ServerCommand *>(to)->MergeFrom(
      static_cast<const ServerCommand &>(from));
}


void ServerCommand::MergeFrom(const ServerCommand& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:opentera.protobuf.ServerCommand)
  GOOGLE_DCHECK_NE(&from, this);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from._internal_has_data()) {
    _internal_mutable_data()->PROTOBUF_NAMESPACE_ID::Any::MergeFrom(from._internal_data());
  }
  if (from._internal_type() != 0) {
    _internal_set_type(from._internal_type());
  }
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void ServerCommand::CopyFrom(const ServerCommand& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:opentera.protobuf.ServerCommand)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool ServerCommand::IsInitialized() const {
  return true;
}

void ServerCommand::InternalSwap(ServerCommand* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(ServerCommand, type_)
      + sizeof(ServerCommand::type_)
      - PROTOBUF_FIELD_OFFSET(ServerCommand, data_)>(
          reinterpret_cast<char*>(&data_),
          reinterpret_cast<char*>(&other->data_));
}

::PROTOBUF_NAMESPACE_ID::Metadata ServerCommand::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_ServerCommand_2eproto_getter, &descriptor_table_ServerCommand_2eproto_once,
      file_level_metadata_ServerCommand_2eproto[0]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace protobuf
}  // namespace opentera
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::opentera::protobuf::ServerCommand* Arena::CreateMaybeMessage< ::opentera::protobuf::ServerCommand >(Arena* arena) {
  return Arena::CreateMessageInternal< ::opentera::protobuf::ServerCommand >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
