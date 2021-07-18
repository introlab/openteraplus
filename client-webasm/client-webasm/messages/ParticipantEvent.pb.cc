// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: ParticipantEvent.proto

#include "ParticipantEvent.pb.h"

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
constexpr ParticipantEvent::ParticipantEvent(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized)
  : participant_uuid_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string)
  , participant_name_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string)
  , participant_project_name_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string)
  , participant_site_name_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string)
  , type_(0)
{}
struct ParticipantEventDefaultTypeInternal {
  constexpr ParticipantEventDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~ParticipantEventDefaultTypeInternal() {}
  union {
    ParticipantEvent _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT ParticipantEventDefaultTypeInternal _ParticipantEvent_default_instance_;
}  // namespace protobuf
}  // namespace opentera
static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_ParticipantEvent_2eproto[1];
static const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* file_level_enum_descriptors_ParticipantEvent_2eproto[1];
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_ParticipantEvent_2eproto = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32 TableStruct_ParticipantEvent_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::ParticipantEvent, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::ParticipantEvent, participant_uuid_),
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::ParticipantEvent, type_),
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::ParticipantEvent, participant_name_),
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::ParticipantEvent, participant_project_name_),
  PROTOBUF_FIELD_OFFSET(::opentera::protobuf::ParticipantEvent, participant_site_name_),
};
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, sizeof(::opentera::protobuf::ParticipantEvent)},
};

static ::PROTOBUF_NAMESPACE_ID::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::opentera::protobuf::_ParticipantEvent_default_instance_),
};

const char descriptor_table_protodef_ParticipantEvent_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\026ParticipantEvent.proto\022\021opentera.proto"
  "buf\"\222\003\n\020ParticipantEvent\022\030\n\020participant_"
  "uuid\030\001 \001(\t\022;\n\004type\030\002 \001(\0162-.opentera.prot"
  "obuf.ParticipantEvent.EventType\022\030\n\020parti"
  "cipant_name\030\003 \001(\t\022 \n\030participant_project"
  "_name\030\004 \001(\t\022\035\n\025participant_site_name\030\005 \001"
  "(\t\"\313\001\n\tEventType\022\027\n\023PARTICIPANT_UNKNOWN\020"
  "\000\022\031\n\025PARTICIPANT_CONNECTED\020\001\022\034\n\030PARTICIP"
  "ANT_DISCONNECTED\020\002\022\027\n\023PARTICIPANT_DELETE"
  "D\020\003\022\025\n\021PARTICIPANT_ADDED\020\004\022\036\n\032PARTICIPAN"
  "T_JOINED_SESSION\020\005\022\034\n\030PARTICIPANT_LEFT_S"
  "ESSION\020\006b\006proto3"
  ;
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_ParticipantEvent_2eproto_once;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_ParticipantEvent_2eproto = {
  false, false, 456, descriptor_table_protodef_ParticipantEvent_2eproto, "ParticipantEvent.proto", 
  &descriptor_table_ParticipantEvent_2eproto_once, nullptr, 0, 1,
  schemas, file_default_instances, TableStruct_ParticipantEvent_2eproto::offsets,
  file_level_metadata_ParticipantEvent_2eproto, file_level_enum_descriptors_ParticipantEvent_2eproto, file_level_service_descriptors_ParticipantEvent_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable* descriptor_table_ParticipantEvent_2eproto_getter() {
  return &descriptor_table_ParticipantEvent_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY static ::PROTOBUF_NAMESPACE_ID::internal::AddDescriptorsRunner dynamic_init_dummy_ParticipantEvent_2eproto(&descriptor_table_ParticipantEvent_2eproto);
namespace opentera {
namespace protobuf {
const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* ParticipantEvent_EventType_descriptor() {
  ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&descriptor_table_ParticipantEvent_2eproto);
  return file_level_enum_descriptors_ParticipantEvent_2eproto[0];
}
bool ParticipantEvent_EventType_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      return true;
    default:
      return false;
  }
}

#if (__cplusplus < 201703) && (!defined(_MSC_VER) || _MSC_VER >= 1900)
constexpr ParticipantEvent_EventType ParticipantEvent::PARTICIPANT_UNKNOWN;
constexpr ParticipantEvent_EventType ParticipantEvent::PARTICIPANT_CONNECTED;
constexpr ParticipantEvent_EventType ParticipantEvent::PARTICIPANT_DISCONNECTED;
constexpr ParticipantEvent_EventType ParticipantEvent::PARTICIPANT_DELETED;
constexpr ParticipantEvent_EventType ParticipantEvent::PARTICIPANT_ADDED;
constexpr ParticipantEvent_EventType ParticipantEvent::PARTICIPANT_JOINED_SESSION;
constexpr ParticipantEvent_EventType ParticipantEvent::PARTICIPANT_LEFT_SESSION;
constexpr ParticipantEvent_EventType ParticipantEvent::EventType_MIN;
constexpr ParticipantEvent_EventType ParticipantEvent::EventType_MAX;
constexpr int ParticipantEvent::EventType_ARRAYSIZE;
#endif  // (__cplusplus < 201703) && (!defined(_MSC_VER) || _MSC_VER >= 1900)

// ===================================================================

class ParticipantEvent::_Internal {
 public:
};

ParticipantEvent::ParticipantEvent(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor();
  if (!is_message_owned) {
    RegisterArenaDtor(arena);
  }
  // @@protoc_insertion_point(arena_constructor:opentera.protobuf.ParticipantEvent)
}
ParticipantEvent::ParticipantEvent(const ParticipantEvent& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  participant_uuid_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_participant_uuid().empty()) {
    participant_uuid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_participant_uuid(), 
      GetArenaForAllocation());
  }
  participant_name_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_participant_name().empty()) {
    participant_name_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_participant_name(), 
      GetArenaForAllocation());
  }
  participant_project_name_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_participant_project_name().empty()) {
    participant_project_name_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_participant_project_name(), 
      GetArenaForAllocation());
  }
  participant_site_name_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_participant_site_name().empty()) {
    participant_site_name_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_participant_site_name(), 
      GetArenaForAllocation());
  }
  type_ = from.type_;
  // @@protoc_insertion_point(copy_constructor:opentera.protobuf.ParticipantEvent)
}

inline void ParticipantEvent::SharedCtor() {
participant_uuid_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
participant_name_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
participant_project_name_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
participant_site_name_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
type_ = 0;
}

ParticipantEvent::~ParticipantEvent() {
  // @@protoc_insertion_point(destructor:opentera.protobuf.ParticipantEvent)
  if (GetArenaForAllocation() != nullptr) return;
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

inline void ParticipantEvent::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  participant_uuid_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  participant_name_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  participant_project_name_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  participant_site_name_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

void ParticipantEvent::ArenaDtor(void* object) {
  ParticipantEvent* _this = reinterpret_cast< ParticipantEvent* >(object);
  (void)_this;
}
void ParticipantEvent::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void ParticipantEvent::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void ParticipantEvent::Clear() {
// @@protoc_insertion_point(message_clear_start:opentera.protobuf.ParticipantEvent)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  participant_uuid_.ClearToEmpty();
  participant_name_.ClearToEmpty();
  participant_project_name_.ClearToEmpty();
  participant_site_name_.ClearToEmpty();
  type_ = 0;
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* ParticipantEvent::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // string participant_uuid = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 10)) {
          auto str = _internal_mutable_participant_uuid();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "opentera.protobuf.ParticipantEvent.participant_uuid"));
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // .opentera.protobuf.ParticipantEvent.EventType type = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 16)) {
          ::PROTOBUF_NAMESPACE_ID::uint64 val = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
          _internal_set_type(static_cast<::opentera::protobuf::ParticipantEvent_EventType>(val));
        } else goto handle_unusual;
        continue;
      // string participant_name = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 26)) {
          auto str = _internal_mutable_participant_name();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "opentera.protobuf.ParticipantEvent.participant_name"));
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // string participant_project_name = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 34)) {
          auto str = _internal_mutable_participant_project_name();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "opentera.protobuf.ParticipantEvent.participant_project_name"));
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // string participant_site_name = 5;
      case 5:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 42)) {
          auto str = _internal_mutable_participant_site_name();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "opentera.protobuf.ParticipantEvent.participant_site_name"));
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

::PROTOBUF_NAMESPACE_ID::uint8* ParticipantEvent::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:opentera.protobuf.ParticipantEvent)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // string participant_uuid = 1;
  if (!this->_internal_participant_uuid().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_participant_uuid().data(), static_cast<int>(this->_internal_participant_uuid().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "opentera.protobuf.ParticipantEvent.participant_uuid");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_participant_uuid(), target);
  }

  // .opentera.protobuf.ParticipantEvent.EventType type = 2;
  if (this->_internal_type() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteEnumToArray(
      2, this->_internal_type(), target);
  }

  // string participant_name = 3;
  if (!this->_internal_participant_name().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_participant_name().data(), static_cast<int>(this->_internal_participant_name().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "opentera.protobuf.ParticipantEvent.participant_name");
    target = stream->WriteStringMaybeAliased(
        3, this->_internal_participant_name(), target);
  }

  // string participant_project_name = 4;
  if (!this->_internal_participant_project_name().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_participant_project_name().data(), static_cast<int>(this->_internal_participant_project_name().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "opentera.protobuf.ParticipantEvent.participant_project_name");
    target = stream->WriteStringMaybeAliased(
        4, this->_internal_participant_project_name(), target);
  }

  // string participant_site_name = 5;
  if (!this->_internal_participant_site_name().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_participant_site_name().data(), static_cast<int>(this->_internal_participant_site_name().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "opentera.protobuf.ParticipantEvent.participant_site_name");
    target = stream->WriteStringMaybeAliased(
        5, this->_internal_participant_site_name(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:opentera.protobuf.ParticipantEvent)
  return target;
}

size_t ParticipantEvent::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:opentera.protobuf.ParticipantEvent)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // string participant_uuid = 1;
  if (!this->_internal_participant_uuid().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_participant_uuid());
  }

  // string participant_name = 3;
  if (!this->_internal_participant_name().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_participant_name());
  }

  // string participant_project_name = 4;
  if (!this->_internal_participant_project_name().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_participant_project_name());
  }

  // string participant_site_name = 5;
  if (!this->_internal_participant_site_name().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_participant_site_name());
  }

  // .opentera.protobuf.ParticipantEvent.EventType type = 2;
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

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData ParticipantEvent::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    ParticipantEvent::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*ParticipantEvent::GetClassData() const { return &_class_data_; }

void ParticipantEvent::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message*to,
                      const ::PROTOBUF_NAMESPACE_ID::Message&from) {
  static_cast<ParticipantEvent *>(to)->MergeFrom(
      static_cast<const ParticipantEvent &>(from));
}


void ParticipantEvent::MergeFrom(const ParticipantEvent& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:opentera.protobuf.ParticipantEvent)
  GOOGLE_DCHECK_NE(&from, this);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_participant_uuid().empty()) {
    _internal_set_participant_uuid(from._internal_participant_uuid());
  }
  if (!from._internal_participant_name().empty()) {
    _internal_set_participant_name(from._internal_participant_name());
  }
  if (!from._internal_participant_project_name().empty()) {
    _internal_set_participant_project_name(from._internal_participant_project_name());
  }
  if (!from._internal_participant_site_name().empty()) {
    _internal_set_participant_site_name(from._internal_participant_site_name());
  }
  if (from._internal_type() != 0) {
    _internal_set_type(from._internal_type());
  }
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void ParticipantEvent::CopyFrom(const ParticipantEvent& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:opentera.protobuf.ParticipantEvent)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool ParticipantEvent::IsInitialized() const {
  return true;
}

void ParticipantEvent::InternalSwap(ParticipantEvent* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &participant_uuid_, GetArenaForAllocation(),
      &other->participant_uuid_, other->GetArenaForAllocation()
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &participant_name_, GetArenaForAllocation(),
      &other->participant_name_, other->GetArenaForAllocation()
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &participant_project_name_, GetArenaForAllocation(),
      &other->participant_project_name_, other->GetArenaForAllocation()
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &participant_site_name_, GetArenaForAllocation(),
      &other->participant_site_name_, other->GetArenaForAllocation()
  );
  swap(type_, other->type_);
}

::PROTOBUF_NAMESPACE_ID::Metadata ParticipantEvent::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_ParticipantEvent_2eproto_getter, &descriptor_table_ParticipantEvent_2eproto_once,
      file_level_metadata_ParticipantEvent_2eproto[0]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace protobuf
}  // namespace opentera
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::opentera::protobuf::ParticipantEvent* Arena::CreateMaybeMessage< ::opentera::protobuf::ParticipantEvent >(Arena* arena) {
  return Arena::CreateMessageInternal< ::opentera::protobuf::ParticipantEvent >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
