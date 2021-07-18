// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: TeraModuleMessage.proto

#include "TeraModuleMessage.pb.h"

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
constexpr TeraModuleMessage_Header::TeraModuleMessage_Header(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized)
  : source_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string)
  , dest_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string)
  , time_(0)
  , version_(0u)
  , seq_(0u){}
struct TeraModuleMessage_HeaderDefaultTypeInternal {
  constexpr TeraModuleMessage_HeaderDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~TeraModuleMessage_HeaderDefaultTypeInternal() {}
  union {
    TeraModuleMessage_Header _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT TeraModuleMessage_HeaderDefaultTypeInternal _TeraModuleMessage_Header_default_instance_;
constexpr TeraModuleMessage::TeraModuleMessage(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized)
  : data_()
  , head_(nullptr){}
struct TeraModuleMessageDefaultTypeInternal {
  constexpr TeraModuleMessageDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~TeraModuleMessageDefaultTypeInternal() {}
  union {
    TeraModuleMessage _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT TeraModuleMessageDefaultTypeInternal _TeraModuleMessage_default_instance_;
}  // namespace protobuf
}  // namespace opentera
static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_TeraModuleMessage_2eproto[2];
static constexpr ::PROTOBUF_NAMESPACE_ID::EnumDescriptor const** file_level_enum_descriptors_TeraModuleMessage_2eproto = nullptr;
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_TeraModuleMessage_2eproto = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32 TableStruct_TeraModuleMessage_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::TeraModuleMessage_Header, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::TeraModuleMessage_Header, version_),
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::TeraModuleMessage_Header, time_),
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::TeraModuleMessage_Header, seq_),
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::TeraModuleMessage_Header, source_),
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::TeraModuleMessage_Header, dest_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::TeraModuleMessage, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::TeraModuleMessage, head_),
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::TeraModuleMessage, data_),
};
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, sizeof(::opentera::protobuf::TeraModuleMessage_Header)},
  { 10, -1, sizeof(::opentera::protobuf::TeraModuleMessage)},
};

static ::PROTOBUF_NAMESPACE_ID::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::opentera::protobuf::_TeraModuleMessage_Header_default_instance_),
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::opentera::protobuf::_TeraModuleMessage_default_instance_),
};

const char descriptor_table_protodef_TeraModuleMessage_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\027TeraModuleMessage.proto\022\021opentera.prot"
  "obuf\032\031google/protobuf/any.proto\"\306\001\n\021Tera"
  "ModuleMessage\0229\n\004head\030\001 \001(\0132+.opentera.p"
  "rotobuf.TeraModuleMessage.Header\022\"\n\004data"
  "\030\002 \003(\0132\024.google.protobuf.Any\032R\n\006Header\022\017"
  "\n\007version\030\001 \001(\r\022\014\n\004time\030\002 \001(\001\022\013\n\003seq\030\003 \001"
  "(\r\022\016\n\006source\030\004 \001(\t\022\014\n\004dest\030\005 \001(\tb\006proto3"
  ;
static const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable*const descriptor_table_TeraModuleMessage_2eproto_deps[1] = {
  &::descriptor_table_google_2fprotobuf_2fany_2eproto,
};
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_TeraModuleMessage_2eproto_once;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_TeraModuleMessage_2eproto = {
  false, false, 280, descriptor_table_protodef_TeraModuleMessage_2eproto, "TeraModuleMessage.proto", 
  &descriptor_table_TeraModuleMessage_2eproto_once, descriptor_table_TeraModuleMessage_2eproto_deps, 1, 2,
  schemas, file_default_instances, TableStruct_TeraModuleMessage_2eproto::offsets,
  file_level_metadata_TeraModuleMessage_2eproto, file_level_enum_descriptors_TeraModuleMessage_2eproto, file_level_service_descriptors_TeraModuleMessage_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable* descriptor_table_TeraModuleMessage_2eproto_getter() {
  return &descriptor_table_TeraModuleMessage_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY static ::PROTOBUF_NAMESPACE_ID::internal::AddDescriptorsRunner dynamic_init_dummy_TeraModuleMessage_2eproto(&descriptor_table_TeraModuleMessage_2eproto);
namespace opentera {
namespace protobuf {

// ===================================================================

class TeraModuleMessage_Header::_Internal {
 public:
};

TeraModuleMessage_Header::TeraModuleMessage_Header(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor();
  if (!is_message_owned) {
    RegisterArenaDtor(arena);
  }
  // @@protoc_insertion_point(arena_constructor:opentera.protobuf.TeraModuleMessage.Header)
}
TeraModuleMessage_Header::TeraModuleMessage_Header(const TeraModuleMessage_Header& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  source_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_source().empty()) {
    source_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_source(), 
      GetArenaForAllocation());
  }
  dest_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_dest().empty()) {
    dest_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_dest(), 
      GetArenaForAllocation());
  }
  ::memcpy(&time_, &from.time_,
    static_cast<size_t>(reinterpret_cast<char*>(&seq_) -
    reinterpret_cast<char*>(&time_)) + sizeof(seq_));
  // @@protoc_insertion_point(copy_constructor:opentera.protobuf.TeraModuleMessage.Header)
}

inline void TeraModuleMessage_Header::SharedCtor() {
source_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
dest_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
::memset(reinterpret_cast<char*>(this) + static_cast<size_t>(
    reinterpret_cast<char*>(&time_) - reinterpret_cast<char*>(this)),
    0, static_cast<size_t>(reinterpret_cast<char*>(&seq_) -
    reinterpret_cast<char*>(&time_)) + sizeof(seq_));
}

TeraModuleMessage_Header::~TeraModuleMessage_Header() {
  // @@protoc_insertion_point(destructor:opentera.protobuf.TeraModuleMessage.Header)
  if (GetArenaForAllocation() != nullptr) return;
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

inline void TeraModuleMessage_Header::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  source_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  dest_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

void TeraModuleMessage_Header::ArenaDtor(void* object) {
  TeraModuleMessage_Header* _this = reinterpret_cast< TeraModuleMessage_Header* >(object);
  (void)_this;
}
void TeraModuleMessage_Header::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void TeraModuleMessage_Header::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void TeraModuleMessage_Header::Clear() {
// @@protoc_insertion_point(message_clear_start:opentera.protobuf.TeraModuleMessage.Header)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  source_.ClearToEmpty();
  dest_.ClearToEmpty();
  ::memset(&time_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&seq_) -
      reinterpret_cast<char*>(&time_)) + sizeof(seq_));
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* TeraModuleMessage_Header::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // uint32 version = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 8)) {
          version_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint32(&ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // double time = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 17)) {
          time_ = ::PROTOBUF_NAMESPACE_ID::internal::UnalignedLoad<double>(ptr);
          ptr += sizeof(double);
        } else goto handle_unusual;
        continue;
      // uint32 seq = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 24)) {
          seq_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint32(&ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // string source = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 34)) {
          auto str = _internal_mutable_source();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "opentera.protobuf.TeraModuleMessage.Header.source"));
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // string dest = 5;
      case 5:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 42)) {
          auto str = _internal_mutable_dest();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "opentera.protobuf.TeraModuleMessage.Header.dest"));
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

::PROTOBUF_NAMESPACE_ID::uint8* TeraModuleMessage_Header::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:opentera.protobuf.TeraModuleMessage.Header)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // uint32 version = 1;
  if (this->_internal_version() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteUInt32ToArray(1, this->_internal_version(), target);
  }

  // double time = 2;
  if (!(this->_internal_time() <= 0 && this->_internal_time() >= 0)) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteDoubleToArray(2, this->_internal_time(), target);
  }

  // uint32 seq = 3;
  if (this->_internal_seq() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteUInt32ToArray(3, this->_internal_seq(), target);
  }

  // string source = 4;
  if (!this->_internal_source().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_source().data(), static_cast<int>(this->_internal_source().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "opentera.protobuf.TeraModuleMessage.Header.source");
    target = stream->WriteStringMaybeAliased(
        4, this->_internal_source(), target);
  }

  // string dest = 5;
  if (!this->_internal_dest().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_dest().data(), static_cast<int>(this->_internal_dest().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "opentera.protobuf.TeraModuleMessage.Header.dest");
    target = stream->WriteStringMaybeAliased(
        5, this->_internal_dest(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:opentera.protobuf.TeraModuleMessage.Header)
  return target;
}

size_t TeraModuleMessage_Header::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:opentera.protobuf.TeraModuleMessage.Header)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // string source = 4;
  if (!this->_internal_source().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_source());
  }

  // string dest = 5;
  if (!this->_internal_dest().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_dest());
  }

  // double time = 2;
  if (!(this->_internal_time() <= 0 && this->_internal_time() >= 0)) {
    total_size += 1 + 8;
  }

  // uint32 version = 1;
  if (this->_internal_version() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::UInt32Size(
        this->_internal_version());
  }

  // uint32 seq = 3;
  if (this->_internal_seq() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::UInt32Size(
        this->_internal_seq());
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(
        _internal_metadata_, total_size, &_cached_size_);
  }
  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData TeraModuleMessage_Header::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    TeraModuleMessage_Header::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*TeraModuleMessage_Header::GetClassData() const { return &_class_data_; }

void TeraModuleMessage_Header::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message*to,
                      const ::PROTOBUF_NAMESPACE_ID::Message&from) {
  static_cast<TeraModuleMessage_Header *>(to)->MergeFrom(
      static_cast<const TeraModuleMessage_Header &>(from));
}


void TeraModuleMessage_Header::MergeFrom(const TeraModuleMessage_Header& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:opentera.protobuf.TeraModuleMessage.Header)
  GOOGLE_DCHECK_NE(&from, this);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_source().empty()) {
    _internal_set_source(from._internal_source());
  }
  if (!from._internal_dest().empty()) {
    _internal_set_dest(from._internal_dest());
  }
  if (!(from._internal_time() <= 0 && from._internal_time() >= 0)) {
    _internal_set_time(from._internal_time());
  }
  if (from._internal_version() != 0) {
    _internal_set_version(from._internal_version());
  }
  if (from._internal_seq() != 0) {
    _internal_set_seq(from._internal_seq());
  }
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void TeraModuleMessage_Header::CopyFrom(const TeraModuleMessage_Header& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:opentera.protobuf.TeraModuleMessage.Header)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool TeraModuleMessage_Header::IsInitialized() const {
  return true;
}

void TeraModuleMessage_Header::InternalSwap(TeraModuleMessage_Header* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &source_, GetArenaForAllocation(),
      &other->source_, other->GetArenaForAllocation()
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &dest_, GetArenaForAllocation(),
      &other->dest_, other->GetArenaForAllocation()
  );
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(TeraModuleMessage_Header, seq_)
      + sizeof(TeraModuleMessage_Header::seq_)
      - PROTOBUF_FIELD_OFFSET(TeraModuleMessage_Header, time_)>(
          reinterpret_cast<char*>(&time_),
          reinterpret_cast<char*>(&other->time_));
}

::PROTOBUF_NAMESPACE_ID::Metadata TeraModuleMessage_Header::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_TeraModuleMessage_2eproto_getter, &descriptor_table_TeraModuleMessage_2eproto_once,
      file_level_metadata_TeraModuleMessage_2eproto[0]);
}

// ===================================================================

class TeraModuleMessage::_Internal {
 public:
  static const ::opentera::protobuf::TeraModuleMessage_Header& head(const TeraModuleMessage* msg);
};

const ::opentera::protobuf::TeraModuleMessage_Header&
TeraModuleMessage::_Internal::head(const TeraModuleMessage* msg) {
  return *msg->head_;
}
void TeraModuleMessage::clear_data() {
  data_.Clear();
}
TeraModuleMessage::TeraModuleMessage(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned),
  data_(arena) {
  SharedCtor();
  if (!is_message_owned) {
    RegisterArenaDtor(arena);
  }
  // @@protoc_insertion_point(arena_constructor:opentera.protobuf.TeraModuleMessage)
}
TeraModuleMessage::TeraModuleMessage(const TeraModuleMessage& from)
  : ::PROTOBUF_NAMESPACE_ID::Message(),
      data_(from.data_) {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  if (from._internal_has_head()) {
    head_ = new ::opentera::protobuf::TeraModuleMessage_Header(*from.head_);
  } else {
    head_ = nullptr;
  }
  // @@protoc_insertion_point(copy_constructor:opentera.protobuf.TeraModuleMessage)
}

inline void TeraModuleMessage::SharedCtor() {
head_ = nullptr;
}

TeraModuleMessage::~TeraModuleMessage() {
  // @@protoc_insertion_point(destructor:opentera.protobuf.TeraModuleMessage)
  if (GetArenaForAllocation() != nullptr) return;
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

inline void TeraModuleMessage::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  if (this != internal_default_instance()) delete head_;
}

void TeraModuleMessage::ArenaDtor(void* object) {
  TeraModuleMessage* _this = reinterpret_cast< TeraModuleMessage* >(object);
  (void)_this;
}
void TeraModuleMessage::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void TeraModuleMessage::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void TeraModuleMessage::Clear() {
// @@protoc_insertion_point(message_clear_start:opentera.protobuf.TeraModuleMessage)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  data_.Clear();
  if (GetArenaForAllocation() == nullptr && head_ != nullptr) {
    delete head_;
  }
  head_ = nullptr;
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* TeraModuleMessage::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // .opentera.protobuf.TeraModuleMessage.Header head = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 10)) {
          ptr = ctx->ParseMessage(_internal_mutable_head(), ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // repeated .google.protobuf.Any data = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 18)) {
          ptr -= 1;
          do {
            ptr += 1;
            ptr = ctx->ParseMessage(_internal_add_data(), ptr);
            CHK_(ptr);
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<18>(ptr));
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

::PROTOBUF_NAMESPACE_ID::uint8* TeraModuleMessage::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:opentera.protobuf.TeraModuleMessage)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // .opentera.protobuf.TeraModuleMessage.Header head = 1;
  if (this->_internal_has_head()) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessage(
        1, _Internal::head(this), target, stream);
  }

  // repeated .google.protobuf.Any data = 2;
  for (unsigned int i = 0,
      n = static_cast<unsigned int>(this->_internal_data_size()); i < n; i++) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessage(2, this->_internal_data(i), target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:opentera.protobuf.TeraModuleMessage)
  return target;
}

size_t TeraModuleMessage::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:opentera.protobuf.TeraModuleMessage)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated .google.protobuf.Any data = 2;
  total_size += 1UL * this->_internal_data_size();
  for (const auto& msg : this->data_) {
    total_size +=
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(msg);
  }

  // .opentera.protobuf.TeraModuleMessage.Header head = 1;
  if (this->_internal_has_head()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(
        *head_);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(
        _internal_metadata_, total_size, &_cached_size_);
  }
  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData TeraModuleMessage::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    TeraModuleMessage::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*TeraModuleMessage::GetClassData() const { return &_class_data_; }

void TeraModuleMessage::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message*to,
                      const ::PROTOBUF_NAMESPACE_ID::Message&from) {
  static_cast<TeraModuleMessage *>(to)->MergeFrom(
      static_cast<const TeraModuleMessage &>(from));
}


void TeraModuleMessage::MergeFrom(const TeraModuleMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:opentera.protobuf.TeraModuleMessage)
  GOOGLE_DCHECK_NE(&from, this);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  data_.MergeFrom(from.data_);
  if (from._internal_has_head()) {
    _internal_mutable_head()->::opentera::protobuf::TeraModuleMessage_Header::MergeFrom(from._internal_head());
  }
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void TeraModuleMessage::CopyFrom(const TeraModuleMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:opentera.protobuf.TeraModuleMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool TeraModuleMessage::IsInitialized() const {
  return true;
}

void TeraModuleMessage::InternalSwap(TeraModuleMessage* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  data_.InternalSwap(&other->data_);
  swap(head_, other->head_);
}

::PROTOBUF_NAMESPACE_ID::Metadata TeraModuleMessage::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_TeraModuleMessage_2eproto_getter, &descriptor_table_TeraModuleMessage_2eproto_once,
      file_level_metadata_TeraModuleMessage_2eproto[1]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace protobuf
}  // namespace opentera
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::opentera::protobuf::TeraModuleMessage_Header* Arena::CreateMaybeMessage< ::opentera::protobuf::TeraModuleMessage_Header >(Arena* arena) {
  return Arena::CreateMessageInternal< ::opentera::protobuf::TeraModuleMessage_Header >(arena);
}
template<> PROTOBUF_NOINLINE ::opentera::protobuf::TeraModuleMessage* Arena::CreateMaybeMessage< ::opentera::protobuf::TeraModuleMessage >(Arena* arena) {
  return Arena::CreateMessageInternal< ::opentera::protobuf::TeraModuleMessage >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
