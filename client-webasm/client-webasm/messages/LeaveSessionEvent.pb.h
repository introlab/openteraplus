// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: LeaveSessionEvent.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_LeaveSessionEvent_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_LeaveSessionEvent_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3017000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3017003 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_LeaveSessionEvent_2eproto OPENTERAMESSAGES_EXPORT
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct OPENTERAMESSAGES_EXPORT TableStruct_LeaveSessionEvent_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxiliaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
OPENTERAMESSAGES_EXPORT extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_LeaveSessionEvent_2eproto;
namespace opentera {
namespace protobuf {
class LeaveSessionEvent;
struct LeaveSessionEventDefaultTypeInternal;
OPENTERAMESSAGES_EXPORT extern LeaveSessionEventDefaultTypeInternal _LeaveSessionEvent_default_instance_;
}  // namespace protobuf
}  // namespace opentera
PROTOBUF_NAMESPACE_OPEN
template<> OPENTERAMESSAGES_EXPORT ::opentera::protobuf::LeaveSessionEvent* Arena::CreateMaybeMessage<::opentera::protobuf::LeaveSessionEvent>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace opentera {
namespace protobuf {

// ===================================================================

class OPENTERAMESSAGES_EXPORT LeaveSessionEvent final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:opentera.protobuf.LeaveSessionEvent) */ {
 public:
  inline LeaveSessionEvent() : LeaveSessionEvent(nullptr) {}
  ~LeaveSessionEvent() override;
  explicit constexpr LeaveSessionEvent(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  LeaveSessionEvent(const LeaveSessionEvent& from);
  LeaveSessionEvent(LeaveSessionEvent&& from) noexcept
    : LeaveSessionEvent() {
    *this = ::std::move(from);
  }

  inline LeaveSessionEvent& operator=(const LeaveSessionEvent& from) {
    CopyFrom(from);
    return *this;
  }
  inline LeaveSessionEvent& operator=(LeaveSessionEvent&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const LeaveSessionEvent& default_instance() {
    return *internal_default_instance();
  }
  static inline const LeaveSessionEvent* internal_default_instance() {
    return reinterpret_cast<const LeaveSessionEvent*>(
               &_LeaveSessionEvent_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(LeaveSessionEvent& a, LeaveSessionEvent& b) {
    a.Swap(&b);
  }
  inline void Swap(LeaveSessionEvent* other) {
    if (other == this) return;
    if (GetOwningArena() == other->GetOwningArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(LeaveSessionEvent* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline LeaveSessionEvent* New() const final {
    return new LeaveSessionEvent();
  }

  LeaveSessionEvent* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<LeaveSessionEvent>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const LeaveSessionEvent& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const LeaveSessionEvent& from);
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message*to, const ::PROTOBUF_NAMESPACE_ID::Message&from);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(LeaveSessionEvent* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "opentera.protobuf.LeaveSessionEvent";
  }
  protected:
  explicit LeaveSessionEvent(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kLeavingParticipantsFieldNumber = 3,
    kLeavingUsersFieldNumber = 4,
    kLeavingDevicesFieldNumber = 5,
    kSessionUuidFieldNumber = 1,
    kServiceUuidFieldNumber = 2,
  };
  // repeated string leaving_participants = 3;
  int leaving_participants_size() const;
  private:
  int _internal_leaving_participants_size() const;
  public:
  void clear_leaving_participants();
  const std::string& leaving_participants(int index) const;
  std::string* mutable_leaving_participants(int index);
  void set_leaving_participants(int index, const std::string& value);
  void set_leaving_participants(int index, std::string&& value);
  void set_leaving_participants(int index, const char* value);
  void set_leaving_participants(int index, const char* value, size_t size);
  std::string* add_leaving_participants();
  void add_leaving_participants(const std::string& value);
  void add_leaving_participants(std::string&& value);
  void add_leaving_participants(const char* value);
  void add_leaving_participants(const char* value, size_t size);
  const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>& leaving_participants() const;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>* mutable_leaving_participants();
  private:
  const std::string& _internal_leaving_participants(int index) const;
  std::string* _internal_add_leaving_participants();
  public:

  // repeated string leaving_users = 4;
  int leaving_users_size() const;
  private:
  int _internal_leaving_users_size() const;
  public:
  void clear_leaving_users();
  const std::string& leaving_users(int index) const;
  std::string* mutable_leaving_users(int index);
  void set_leaving_users(int index, const std::string& value);
  void set_leaving_users(int index, std::string&& value);
  void set_leaving_users(int index, const char* value);
  void set_leaving_users(int index, const char* value, size_t size);
  std::string* add_leaving_users();
  void add_leaving_users(const std::string& value);
  void add_leaving_users(std::string&& value);
  void add_leaving_users(const char* value);
  void add_leaving_users(const char* value, size_t size);
  const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>& leaving_users() const;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>* mutable_leaving_users();
  private:
  const std::string& _internal_leaving_users(int index) const;
  std::string* _internal_add_leaving_users();
  public:

  // repeated string leaving_devices = 5;
  int leaving_devices_size() const;
  private:
  int _internal_leaving_devices_size() const;
  public:
  void clear_leaving_devices();
  const std::string& leaving_devices(int index) const;
  std::string* mutable_leaving_devices(int index);
  void set_leaving_devices(int index, const std::string& value);
  void set_leaving_devices(int index, std::string&& value);
  void set_leaving_devices(int index, const char* value);
  void set_leaving_devices(int index, const char* value, size_t size);
  std::string* add_leaving_devices();
  void add_leaving_devices(const std::string& value);
  void add_leaving_devices(std::string&& value);
  void add_leaving_devices(const char* value);
  void add_leaving_devices(const char* value, size_t size);
  const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>& leaving_devices() const;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>* mutable_leaving_devices();
  private:
  const std::string& _internal_leaving_devices(int index) const;
  std::string* _internal_add_leaving_devices();
  public:

  // string session_uuid = 1;
  void clear_session_uuid();
  const std::string& session_uuid() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_session_uuid(ArgT0&& arg0, ArgT... args);
  std::string* mutable_session_uuid();
  PROTOBUF_MUST_USE_RESULT std::string* release_session_uuid();
  void set_allocated_session_uuid(std::string* session_uuid);
  private:
  const std::string& _internal_session_uuid() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_session_uuid(const std::string& value);
  std::string* _internal_mutable_session_uuid();
  public:

  // string service_uuid = 2;
  void clear_service_uuid();
  const std::string& service_uuid() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_service_uuid(ArgT0&& arg0, ArgT... args);
  std::string* mutable_service_uuid();
  PROTOBUF_MUST_USE_RESULT std::string* release_service_uuid();
  void set_allocated_service_uuid(std::string* service_uuid);
  private:
  const std::string& _internal_service_uuid() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_service_uuid(const std::string& value);
  std::string* _internal_mutable_service_uuid();
  public:

  // @@protoc_insertion_point(class_scope:opentera.protobuf.LeaveSessionEvent)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string> leaving_participants_;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string> leaving_users_;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string> leaving_devices_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr session_uuid_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr service_uuid_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_LeaveSessionEvent_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// LeaveSessionEvent

// string session_uuid = 1;
inline void LeaveSessionEvent::clear_session_uuid() {
  session_uuid_.ClearToEmpty();
}
inline const std::string& LeaveSessionEvent::session_uuid() const {
  // @@protoc_insertion_point(field_get:opentera.protobuf.LeaveSessionEvent.session_uuid)
  return _internal_session_uuid();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void LeaveSessionEvent::set_session_uuid(ArgT0&& arg0, ArgT... args) {
 
 session_uuid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:opentera.protobuf.LeaveSessionEvent.session_uuid)
}
inline std::string* LeaveSessionEvent::mutable_session_uuid() {
  std::string* _s = _internal_mutable_session_uuid();
  // @@protoc_insertion_point(field_mutable:opentera.protobuf.LeaveSessionEvent.session_uuid)
  return _s;
}
inline const std::string& LeaveSessionEvent::_internal_session_uuid() const {
  return session_uuid_.Get();
}
inline void LeaveSessionEvent::_internal_set_session_uuid(const std::string& value) {
  
  session_uuid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* LeaveSessionEvent::_internal_mutable_session_uuid() {
  
  return session_uuid_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* LeaveSessionEvent::release_session_uuid() {
  // @@protoc_insertion_point(field_release:opentera.protobuf.LeaveSessionEvent.session_uuid)
  return session_uuid_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void LeaveSessionEvent::set_allocated_session_uuid(std::string* session_uuid) {
  if (session_uuid != nullptr) {
    
  } else {
    
  }
  session_uuid_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), session_uuid,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:opentera.protobuf.LeaveSessionEvent.session_uuid)
}

// string service_uuid = 2;
inline void LeaveSessionEvent::clear_service_uuid() {
  service_uuid_.ClearToEmpty();
}
inline const std::string& LeaveSessionEvent::service_uuid() const {
  // @@protoc_insertion_point(field_get:opentera.protobuf.LeaveSessionEvent.service_uuid)
  return _internal_service_uuid();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void LeaveSessionEvent::set_service_uuid(ArgT0&& arg0, ArgT... args) {
 
 service_uuid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:opentera.protobuf.LeaveSessionEvent.service_uuid)
}
inline std::string* LeaveSessionEvent::mutable_service_uuid() {
  std::string* _s = _internal_mutable_service_uuid();
  // @@protoc_insertion_point(field_mutable:opentera.protobuf.LeaveSessionEvent.service_uuid)
  return _s;
}
inline const std::string& LeaveSessionEvent::_internal_service_uuid() const {
  return service_uuid_.Get();
}
inline void LeaveSessionEvent::_internal_set_service_uuid(const std::string& value) {
  
  service_uuid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* LeaveSessionEvent::_internal_mutable_service_uuid() {
  
  return service_uuid_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* LeaveSessionEvent::release_service_uuid() {
  // @@protoc_insertion_point(field_release:opentera.protobuf.LeaveSessionEvent.service_uuid)
  return service_uuid_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void LeaveSessionEvent::set_allocated_service_uuid(std::string* service_uuid) {
  if (service_uuid != nullptr) {
    
  } else {
    
  }
  service_uuid_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), service_uuid,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:opentera.protobuf.LeaveSessionEvent.service_uuid)
}

// repeated string leaving_participants = 3;
inline int LeaveSessionEvent::_internal_leaving_participants_size() const {
  return leaving_participants_.size();
}
inline int LeaveSessionEvent::leaving_participants_size() const {
  return _internal_leaving_participants_size();
}
inline void LeaveSessionEvent::clear_leaving_participants() {
  leaving_participants_.Clear();
}
inline std::string* LeaveSessionEvent::add_leaving_participants() {
  std::string* _s = _internal_add_leaving_participants();
  // @@protoc_insertion_point(field_add_mutable:opentera.protobuf.LeaveSessionEvent.leaving_participants)
  return _s;
}
inline const std::string& LeaveSessionEvent::_internal_leaving_participants(int index) const {
  return leaving_participants_.Get(index);
}
inline const std::string& LeaveSessionEvent::leaving_participants(int index) const {
  // @@protoc_insertion_point(field_get:opentera.protobuf.LeaveSessionEvent.leaving_participants)
  return _internal_leaving_participants(index);
}
inline std::string* LeaveSessionEvent::mutable_leaving_participants(int index) {
  // @@protoc_insertion_point(field_mutable:opentera.protobuf.LeaveSessionEvent.leaving_participants)
  return leaving_participants_.Mutable(index);
}
inline void LeaveSessionEvent::set_leaving_participants(int index, const std::string& value) {
  leaving_participants_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set:opentera.protobuf.LeaveSessionEvent.leaving_participants)
}
inline void LeaveSessionEvent::set_leaving_participants(int index, std::string&& value) {
  leaving_participants_.Mutable(index)->assign(std::move(value));
  // @@protoc_insertion_point(field_set:opentera.protobuf.LeaveSessionEvent.leaving_participants)
}
inline void LeaveSessionEvent::set_leaving_participants(int index, const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  leaving_participants_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:opentera.protobuf.LeaveSessionEvent.leaving_participants)
}
inline void LeaveSessionEvent::set_leaving_participants(int index, const char* value, size_t size) {
  leaving_participants_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:opentera.protobuf.LeaveSessionEvent.leaving_participants)
}
inline std::string* LeaveSessionEvent::_internal_add_leaving_participants() {
  return leaving_participants_.Add();
}
inline void LeaveSessionEvent::add_leaving_participants(const std::string& value) {
  leaving_participants_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:opentera.protobuf.LeaveSessionEvent.leaving_participants)
}
inline void LeaveSessionEvent::add_leaving_participants(std::string&& value) {
  leaving_participants_.Add(std::move(value));
  // @@protoc_insertion_point(field_add:opentera.protobuf.LeaveSessionEvent.leaving_participants)
}
inline void LeaveSessionEvent::add_leaving_participants(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  leaving_participants_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:opentera.protobuf.LeaveSessionEvent.leaving_participants)
}
inline void LeaveSessionEvent::add_leaving_participants(const char* value, size_t size) {
  leaving_participants_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:opentera.protobuf.LeaveSessionEvent.leaving_participants)
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>&
LeaveSessionEvent::leaving_participants() const {
  // @@protoc_insertion_point(field_list:opentera.protobuf.LeaveSessionEvent.leaving_participants)
  return leaving_participants_;
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>*
LeaveSessionEvent::mutable_leaving_participants() {
  // @@protoc_insertion_point(field_mutable_list:opentera.protobuf.LeaveSessionEvent.leaving_participants)
  return &leaving_participants_;
}

// repeated string leaving_users = 4;
inline int LeaveSessionEvent::_internal_leaving_users_size() const {
  return leaving_users_.size();
}
inline int LeaveSessionEvent::leaving_users_size() const {
  return _internal_leaving_users_size();
}
inline void LeaveSessionEvent::clear_leaving_users() {
  leaving_users_.Clear();
}
inline std::string* LeaveSessionEvent::add_leaving_users() {
  std::string* _s = _internal_add_leaving_users();
  // @@protoc_insertion_point(field_add_mutable:opentera.protobuf.LeaveSessionEvent.leaving_users)
  return _s;
}
inline const std::string& LeaveSessionEvent::_internal_leaving_users(int index) const {
  return leaving_users_.Get(index);
}
inline const std::string& LeaveSessionEvent::leaving_users(int index) const {
  // @@protoc_insertion_point(field_get:opentera.protobuf.LeaveSessionEvent.leaving_users)
  return _internal_leaving_users(index);
}
inline std::string* LeaveSessionEvent::mutable_leaving_users(int index) {
  // @@protoc_insertion_point(field_mutable:opentera.protobuf.LeaveSessionEvent.leaving_users)
  return leaving_users_.Mutable(index);
}
inline void LeaveSessionEvent::set_leaving_users(int index, const std::string& value) {
  leaving_users_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set:opentera.protobuf.LeaveSessionEvent.leaving_users)
}
inline void LeaveSessionEvent::set_leaving_users(int index, std::string&& value) {
  leaving_users_.Mutable(index)->assign(std::move(value));
  // @@protoc_insertion_point(field_set:opentera.protobuf.LeaveSessionEvent.leaving_users)
}
inline void LeaveSessionEvent::set_leaving_users(int index, const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  leaving_users_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:opentera.protobuf.LeaveSessionEvent.leaving_users)
}
inline void LeaveSessionEvent::set_leaving_users(int index, const char* value, size_t size) {
  leaving_users_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:opentera.protobuf.LeaveSessionEvent.leaving_users)
}
inline std::string* LeaveSessionEvent::_internal_add_leaving_users() {
  return leaving_users_.Add();
}
inline void LeaveSessionEvent::add_leaving_users(const std::string& value) {
  leaving_users_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:opentera.protobuf.LeaveSessionEvent.leaving_users)
}
inline void LeaveSessionEvent::add_leaving_users(std::string&& value) {
  leaving_users_.Add(std::move(value));
  // @@protoc_insertion_point(field_add:opentera.protobuf.LeaveSessionEvent.leaving_users)
}
inline void LeaveSessionEvent::add_leaving_users(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  leaving_users_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:opentera.protobuf.LeaveSessionEvent.leaving_users)
}
inline void LeaveSessionEvent::add_leaving_users(const char* value, size_t size) {
  leaving_users_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:opentera.protobuf.LeaveSessionEvent.leaving_users)
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>&
LeaveSessionEvent::leaving_users() const {
  // @@protoc_insertion_point(field_list:opentera.protobuf.LeaveSessionEvent.leaving_users)
  return leaving_users_;
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>*
LeaveSessionEvent::mutable_leaving_users() {
  // @@protoc_insertion_point(field_mutable_list:opentera.protobuf.LeaveSessionEvent.leaving_users)
  return &leaving_users_;
}

// repeated string leaving_devices = 5;
inline int LeaveSessionEvent::_internal_leaving_devices_size() const {
  return leaving_devices_.size();
}
inline int LeaveSessionEvent::leaving_devices_size() const {
  return _internal_leaving_devices_size();
}
inline void LeaveSessionEvent::clear_leaving_devices() {
  leaving_devices_.Clear();
}
inline std::string* LeaveSessionEvent::add_leaving_devices() {
  std::string* _s = _internal_add_leaving_devices();
  // @@protoc_insertion_point(field_add_mutable:opentera.protobuf.LeaveSessionEvent.leaving_devices)
  return _s;
}
inline const std::string& LeaveSessionEvent::_internal_leaving_devices(int index) const {
  return leaving_devices_.Get(index);
}
inline const std::string& LeaveSessionEvent::leaving_devices(int index) const {
  // @@protoc_insertion_point(field_get:opentera.protobuf.LeaveSessionEvent.leaving_devices)
  return _internal_leaving_devices(index);
}
inline std::string* LeaveSessionEvent::mutable_leaving_devices(int index) {
  // @@protoc_insertion_point(field_mutable:opentera.protobuf.LeaveSessionEvent.leaving_devices)
  return leaving_devices_.Mutable(index);
}
inline void LeaveSessionEvent::set_leaving_devices(int index, const std::string& value) {
  leaving_devices_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set:opentera.protobuf.LeaveSessionEvent.leaving_devices)
}
inline void LeaveSessionEvent::set_leaving_devices(int index, std::string&& value) {
  leaving_devices_.Mutable(index)->assign(std::move(value));
  // @@protoc_insertion_point(field_set:opentera.protobuf.LeaveSessionEvent.leaving_devices)
}
inline void LeaveSessionEvent::set_leaving_devices(int index, const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  leaving_devices_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:opentera.protobuf.LeaveSessionEvent.leaving_devices)
}
inline void LeaveSessionEvent::set_leaving_devices(int index, const char* value, size_t size) {
  leaving_devices_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:opentera.protobuf.LeaveSessionEvent.leaving_devices)
}
inline std::string* LeaveSessionEvent::_internal_add_leaving_devices() {
  return leaving_devices_.Add();
}
inline void LeaveSessionEvent::add_leaving_devices(const std::string& value) {
  leaving_devices_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:opentera.protobuf.LeaveSessionEvent.leaving_devices)
}
inline void LeaveSessionEvent::add_leaving_devices(std::string&& value) {
  leaving_devices_.Add(std::move(value));
  // @@protoc_insertion_point(field_add:opentera.protobuf.LeaveSessionEvent.leaving_devices)
}
inline void LeaveSessionEvent::add_leaving_devices(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  leaving_devices_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:opentera.protobuf.LeaveSessionEvent.leaving_devices)
}
inline void LeaveSessionEvent::add_leaving_devices(const char* value, size_t size) {
  leaving_devices_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:opentera.protobuf.LeaveSessionEvent.leaving_devices)
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>&
LeaveSessionEvent::leaving_devices() const {
  // @@protoc_insertion_point(field_list:opentera.protobuf.LeaveSessionEvent.leaving_devices)
  return leaving_devices_;
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>*
LeaveSessionEvent::mutable_leaving_devices() {
  // @@protoc_insertion_point(field_mutable_list:opentera.protobuf.LeaveSessionEvent.leaving_devices)
  return &leaving_devices_;
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace protobuf
}  // namespace opentera

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_LeaveSessionEvent_2eproto
