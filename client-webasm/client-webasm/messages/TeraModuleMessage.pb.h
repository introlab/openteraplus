// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: TeraModuleMessage.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_TeraModuleMessage_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_TeraModuleMessage_2eproto

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
#include <google/protobuf/any.pb.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_TeraModuleMessage_2eproto OPENTERAMESSAGES_EXPORT
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct OPENTERAMESSAGES_EXPORT TableStruct_TeraModuleMessage_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxiliaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[2]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
OPENTERAMESSAGES_EXPORT extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_TeraModuleMessage_2eproto;
namespace opentera {
namespace protobuf {
class TeraModuleMessage;
struct TeraModuleMessageDefaultTypeInternal;
OPENTERAMESSAGES_EXPORT extern TeraModuleMessageDefaultTypeInternal _TeraModuleMessage_default_instance_;
class TeraModuleMessage_Header;
struct TeraModuleMessage_HeaderDefaultTypeInternal;
OPENTERAMESSAGES_EXPORT extern TeraModuleMessage_HeaderDefaultTypeInternal _TeraModuleMessage_Header_default_instance_;
}  // namespace protobuf
}  // namespace opentera
PROTOBUF_NAMESPACE_OPEN
template<> OPENTERAMESSAGES_EXPORT ::opentera::protobuf::TeraModuleMessage* Arena::CreateMaybeMessage<::opentera::protobuf::TeraModuleMessage>(Arena*);
template<> OPENTERAMESSAGES_EXPORT ::opentera::protobuf::TeraModuleMessage_Header* Arena::CreateMaybeMessage<::opentera::protobuf::TeraModuleMessage_Header>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace opentera {
namespace protobuf {

// ===================================================================

class OPENTERAMESSAGES_EXPORT TeraModuleMessage_Header final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:opentera.protobuf.TeraModuleMessage.Header) */ {
 public:
  inline TeraModuleMessage_Header() : TeraModuleMessage_Header(nullptr) {}
  ~TeraModuleMessage_Header() override;
  explicit constexpr TeraModuleMessage_Header(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  TeraModuleMessage_Header(const TeraModuleMessage_Header& from);
  TeraModuleMessage_Header(TeraModuleMessage_Header&& from) noexcept
    : TeraModuleMessage_Header() {
    *this = ::std::move(from);
  }

  inline TeraModuleMessage_Header& operator=(const TeraModuleMessage_Header& from) {
    CopyFrom(from);
    return *this;
  }
  inline TeraModuleMessage_Header& operator=(TeraModuleMessage_Header&& from) noexcept {
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
  static const TeraModuleMessage_Header& default_instance() {
    return *internal_default_instance();
  }
  static inline const TeraModuleMessage_Header* internal_default_instance() {
    return reinterpret_cast<const TeraModuleMessage_Header*>(
               &_TeraModuleMessage_Header_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(TeraModuleMessage_Header& a, TeraModuleMessage_Header& b) {
    a.Swap(&b);
  }
  inline void Swap(TeraModuleMessage_Header* other) {
    if (other == this) return;
    if (GetOwningArena() == other->GetOwningArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(TeraModuleMessage_Header* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline TeraModuleMessage_Header* New() const final {
    return new TeraModuleMessage_Header();
  }

  TeraModuleMessage_Header* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<TeraModuleMessage_Header>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const TeraModuleMessage_Header& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const TeraModuleMessage_Header& from);
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
  void InternalSwap(TeraModuleMessage_Header* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "opentera.protobuf.TeraModuleMessage.Header";
  }
  protected:
  explicit TeraModuleMessage_Header(::PROTOBUF_NAMESPACE_ID::Arena* arena,
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
    kSourceFieldNumber = 4,
    kDestFieldNumber = 5,
    kTimeFieldNumber = 2,
    kVersionFieldNumber = 1,
    kSeqFieldNumber = 3,
  };
  // string source = 4;
  void clear_source();
  const std::string& source() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_source(ArgT0&& arg0, ArgT... args);
  std::string* mutable_source();
  PROTOBUF_MUST_USE_RESULT std::string* release_source();
  void set_allocated_source(std::string* source);
  private:
  const std::string& _internal_source() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_source(const std::string& value);
  std::string* _internal_mutable_source();
  public:

  // string dest = 5;
  void clear_dest();
  const std::string& dest() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_dest(ArgT0&& arg0, ArgT... args);
  std::string* mutable_dest();
  PROTOBUF_MUST_USE_RESULT std::string* release_dest();
  void set_allocated_dest(std::string* dest);
  private:
  const std::string& _internal_dest() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_dest(const std::string& value);
  std::string* _internal_mutable_dest();
  public:

  // double time = 2;
  void clear_time();
  double time() const;
  void set_time(double value);
  private:
  double _internal_time() const;
  void _internal_set_time(double value);
  public:

  // uint32 version = 1;
  void clear_version();
  ::PROTOBUF_NAMESPACE_ID::uint32 version() const;
  void set_version(::PROTOBUF_NAMESPACE_ID::uint32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::uint32 _internal_version() const;
  void _internal_set_version(::PROTOBUF_NAMESPACE_ID::uint32 value);
  public:

  // uint32 seq = 3;
  void clear_seq();
  ::PROTOBUF_NAMESPACE_ID::uint32 seq() const;
  void set_seq(::PROTOBUF_NAMESPACE_ID::uint32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::uint32 _internal_seq() const;
  void _internal_set_seq(::PROTOBUF_NAMESPACE_ID::uint32 value);
  public:

  // @@protoc_insertion_point(class_scope:opentera.protobuf.TeraModuleMessage.Header)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr source_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr dest_;
  double time_;
  ::PROTOBUF_NAMESPACE_ID::uint32 version_;
  ::PROTOBUF_NAMESPACE_ID::uint32 seq_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_TeraModuleMessage_2eproto;
};
// -------------------------------------------------------------------

class OPENTERAMESSAGES_EXPORT TeraModuleMessage final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:opentera.protobuf.TeraModuleMessage) */ {
 public:
  inline TeraModuleMessage() : TeraModuleMessage(nullptr) {}
  ~TeraModuleMessage() override;
  explicit constexpr TeraModuleMessage(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  TeraModuleMessage(const TeraModuleMessage& from);
  TeraModuleMessage(TeraModuleMessage&& from) noexcept
    : TeraModuleMessage() {
    *this = ::std::move(from);
  }

  inline TeraModuleMessage& operator=(const TeraModuleMessage& from) {
    CopyFrom(from);
    return *this;
  }
  inline TeraModuleMessage& operator=(TeraModuleMessage&& from) noexcept {
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
  static const TeraModuleMessage& default_instance() {
    return *internal_default_instance();
  }
  static inline const TeraModuleMessage* internal_default_instance() {
    return reinterpret_cast<const TeraModuleMessage*>(
               &_TeraModuleMessage_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(TeraModuleMessage& a, TeraModuleMessage& b) {
    a.Swap(&b);
  }
  inline void Swap(TeraModuleMessage* other) {
    if (other == this) return;
    if (GetOwningArena() == other->GetOwningArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(TeraModuleMessage* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline TeraModuleMessage* New() const final {
    return new TeraModuleMessage();
  }

  TeraModuleMessage* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<TeraModuleMessage>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const TeraModuleMessage& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const TeraModuleMessage& from);
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
  void InternalSwap(TeraModuleMessage* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "opentera.protobuf.TeraModuleMessage";
  }
  protected:
  explicit TeraModuleMessage(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  typedef TeraModuleMessage_Header Header;

  // accessors -------------------------------------------------------

  enum : int {
    kDataFieldNumber = 2,
    kHeadFieldNumber = 1,
  };
  // repeated .google.protobuf.Any data = 2;
  int data_size() const;
  private:
  int _internal_data_size() const;
  public:
  void clear_data();
  PROTOBUF_NAMESPACE_ID::Any* mutable_data(int index);
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< PROTOBUF_NAMESPACE_ID::Any >*
      mutable_data();
  private:
  const PROTOBUF_NAMESPACE_ID::Any& _internal_data(int index) const;
  PROTOBUF_NAMESPACE_ID::Any* _internal_add_data();
  public:
  const PROTOBUF_NAMESPACE_ID::Any& data(int index) const;
  PROTOBUF_NAMESPACE_ID::Any* add_data();
  const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< PROTOBUF_NAMESPACE_ID::Any >&
      data() const;

  // .opentera.protobuf.TeraModuleMessage.Header head = 1;
  bool has_head() const;
  private:
  bool _internal_has_head() const;
  public:
  void clear_head();
  const ::opentera::protobuf::TeraModuleMessage_Header& head() const;
  PROTOBUF_MUST_USE_RESULT ::opentera::protobuf::TeraModuleMessage_Header* release_head();
  ::opentera::protobuf::TeraModuleMessage_Header* mutable_head();
  void set_allocated_head(::opentera::protobuf::TeraModuleMessage_Header* head);
  private:
  const ::opentera::protobuf::TeraModuleMessage_Header& _internal_head() const;
  ::opentera::protobuf::TeraModuleMessage_Header* _internal_mutable_head();
  public:
  void unsafe_arena_set_allocated_head(
      ::opentera::protobuf::TeraModuleMessage_Header* head);
  ::opentera::protobuf::TeraModuleMessage_Header* unsafe_arena_release_head();

  // @@protoc_insertion_point(class_scope:opentera.protobuf.TeraModuleMessage)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< PROTOBUF_NAMESPACE_ID::Any > data_;
  ::opentera::protobuf::TeraModuleMessage_Header* head_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_TeraModuleMessage_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// TeraModuleMessage_Header

// uint32 version = 1;
inline void TeraModuleMessage_Header::clear_version() {
  version_ = 0u;
}
inline ::PROTOBUF_NAMESPACE_ID::uint32 TeraModuleMessage_Header::_internal_version() const {
  return version_;
}
inline ::PROTOBUF_NAMESPACE_ID::uint32 TeraModuleMessage_Header::version() const {
  // @@protoc_insertion_point(field_get:opentera.protobuf.TeraModuleMessage.Header.version)
  return _internal_version();
}
inline void TeraModuleMessage_Header::_internal_set_version(::PROTOBUF_NAMESPACE_ID::uint32 value) {
  
  version_ = value;
}
inline void TeraModuleMessage_Header::set_version(::PROTOBUF_NAMESPACE_ID::uint32 value) {
  _internal_set_version(value);
  // @@protoc_insertion_point(field_set:opentera.protobuf.TeraModuleMessage.Header.version)
}

// double time = 2;
inline void TeraModuleMessage_Header::clear_time() {
  time_ = 0;
}
inline double TeraModuleMessage_Header::_internal_time() const {
  return time_;
}
inline double TeraModuleMessage_Header::time() const {
  // @@protoc_insertion_point(field_get:opentera.protobuf.TeraModuleMessage.Header.time)
  return _internal_time();
}
inline void TeraModuleMessage_Header::_internal_set_time(double value) {
  
  time_ = value;
}
inline void TeraModuleMessage_Header::set_time(double value) {
  _internal_set_time(value);
  // @@protoc_insertion_point(field_set:opentera.protobuf.TeraModuleMessage.Header.time)
}

// uint32 seq = 3;
inline void TeraModuleMessage_Header::clear_seq() {
  seq_ = 0u;
}
inline ::PROTOBUF_NAMESPACE_ID::uint32 TeraModuleMessage_Header::_internal_seq() const {
  return seq_;
}
inline ::PROTOBUF_NAMESPACE_ID::uint32 TeraModuleMessage_Header::seq() const {
  // @@protoc_insertion_point(field_get:opentera.protobuf.TeraModuleMessage.Header.seq)
  return _internal_seq();
}
inline void TeraModuleMessage_Header::_internal_set_seq(::PROTOBUF_NAMESPACE_ID::uint32 value) {
  
  seq_ = value;
}
inline void TeraModuleMessage_Header::set_seq(::PROTOBUF_NAMESPACE_ID::uint32 value) {
  _internal_set_seq(value);
  // @@protoc_insertion_point(field_set:opentera.protobuf.TeraModuleMessage.Header.seq)
}

// string source = 4;
inline void TeraModuleMessage_Header::clear_source() {
  source_.ClearToEmpty();
}
inline const std::string& TeraModuleMessage_Header::source() const {
  // @@protoc_insertion_point(field_get:opentera.protobuf.TeraModuleMessage.Header.source)
  return _internal_source();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void TeraModuleMessage_Header::set_source(ArgT0&& arg0, ArgT... args) {
 
 source_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:opentera.protobuf.TeraModuleMessage.Header.source)
}
inline std::string* TeraModuleMessage_Header::mutable_source() {
  std::string* _s = _internal_mutable_source();
  // @@protoc_insertion_point(field_mutable:opentera.protobuf.TeraModuleMessage.Header.source)
  return _s;
}
inline const std::string& TeraModuleMessage_Header::_internal_source() const {
  return source_.Get();
}
inline void TeraModuleMessage_Header::_internal_set_source(const std::string& value) {
  
  source_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* TeraModuleMessage_Header::_internal_mutable_source() {
  
  return source_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* TeraModuleMessage_Header::release_source() {
  // @@protoc_insertion_point(field_release:opentera.protobuf.TeraModuleMessage.Header.source)
  return source_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void TeraModuleMessage_Header::set_allocated_source(std::string* source) {
  if (source != nullptr) {
    
  } else {
    
  }
  source_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), source,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:opentera.protobuf.TeraModuleMessage.Header.source)
}

// string dest = 5;
inline void TeraModuleMessage_Header::clear_dest() {
  dest_.ClearToEmpty();
}
inline const std::string& TeraModuleMessage_Header::dest() const {
  // @@protoc_insertion_point(field_get:opentera.protobuf.TeraModuleMessage.Header.dest)
  return _internal_dest();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void TeraModuleMessage_Header::set_dest(ArgT0&& arg0, ArgT... args) {
 
 dest_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:opentera.protobuf.TeraModuleMessage.Header.dest)
}
inline std::string* TeraModuleMessage_Header::mutable_dest() {
  std::string* _s = _internal_mutable_dest();
  // @@protoc_insertion_point(field_mutable:opentera.protobuf.TeraModuleMessage.Header.dest)
  return _s;
}
inline const std::string& TeraModuleMessage_Header::_internal_dest() const {
  return dest_.Get();
}
inline void TeraModuleMessage_Header::_internal_set_dest(const std::string& value) {
  
  dest_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* TeraModuleMessage_Header::_internal_mutable_dest() {
  
  return dest_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* TeraModuleMessage_Header::release_dest() {
  // @@protoc_insertion_point(field_release:opentera.protobuf.TeraModuleMessage.Header.dest)
  return dest_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void TeraModuleMessage_Header::set_allocated_dest(std::string* dest) {
  if (dest != nullptr) {
    
  } else {
    
  }
  dest_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), dest,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:opentera.protobuf.TeraModuleMessage.Header.dest)
}

// -------------------------------------------------------------------

// TeraModuleMessage

// .opentera.protobuf.TeraModuleMessage.Header head = 1;
inline bool TeraModuleMessage::_internal_has_head() const {
  return this != internal_default_instance() && head_ != nullptr;
}
inline bool TeraModuleMessage::has_head() const {
  return _internal_has_head();
}
inline void TeraModuleMessage::clear_head() {
  if (GetArenaForAllocation() == nullptr && head_ != nullptr) {
    delete head_;
  }
  head_ = nullptr;
}
inline const ::opentera::protobuf::TeraModuleMessage_Header& TeraModuleMessage::_internal_head() const {
  const ::opentera::protobuf::TeraModuleMessage_Header* p = head_;
  return p != nullptr ? *p : reinterpret_cast<const ::opentera::protobuf::TeraModuleMessage_Header&>(
      ::opentera::protobuf::_TeraModuleMessage_Header_default_instance_);
}
inline const ::opentera::protobuf::TeraModuleMessage_Header& TeraModuleMessage::head() const {
  // @@protoc_insertion_point(field_get:opentera.protobuf.TeraModuleMessage.head)
  return _internal_head();
}
inline void TeraModuleMessage::unsafe_arena_set_allocated_head(
    ::opentera::protobuf::TeraModuleMessage_Header* head) {
  if (GetArenaForAllocation() == nullptr) {
    delete reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(head_);
  }
  head_ = head;
  if (head) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:opentera.protobuf.TeraModuleMessage.head)
}
inline ::opentera::protobuf::TeraModuleMessage_Header* TeraModuleMessage::release_head() {
  
  ::opentera::protobuf::TeraModuleMessage_Header* temp = head_;
  head_ = nullptr;
#ifdef PROTOBUF_FORCE_COPY_IN_RELEASE
  auto* old =  reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(temp);
  temp = ::PROTOBUF_NAMESPACE_ID::internal::DuplicateIfNonNull(temp);
  if (GetArenaForAllocation() == nullptr) { delete old; }
#else  // PROTOBUF_FORCE_COPY_IN_RELEASE
  if (GetArenaForAllocation() != nullptr) {
    temp = ::PROTOBUF_NAMESPACE_ID::internal::DuplicateIfNonNull(temp);
  }
#endif  // !PROTOBUF_FORCE_COPY_IN_RELEASE
  return temp;
}
inline ::opentera::protobuf::TeraModuleMessage_Header* TeraModuleMessage::unsafe_arena_release_head() {
  // @@protoc_insertion_point(field_release:opentera.protobuf.TeraModuleMessage.head)
  
  ::opentera::protobuf::TeraModuleMessage_Header* temp = head_;
  head_ = nullptr;
  return temp;
}
inline ::opentera::protobuf::TeraModuleMessage_Header* TeraModuleMessage::_internal_mutable_head() {
  
  if (head_ == nullptr) {
    auto* p = CreateMaybeMessage<::opentera::protobuf::TeraModuleMessage_Header>(GetArenaForAllocation());
    head_ = p;
  }
  return head_;
}
inline ::opentera::protobuf::TeraModuleMessage_Header* TeraModuleMessage::mutable_head() {
  ::opentera::protobuf::TeraModuleMessage_Header* _msg = _internal_mutable_head();
  // @@protoc_insertion_point(field_mutable:opentera.protobuf.TeraModuleMessage.head)
  return _msg;
}
inline void TeraModuleMessage::set_allocated_head(::opentera::protobuf::TeraModuleMessage_Header* head) {
  ::PROTOBUF_NAMESPACE_ID::Arena* message_arena = GetArenaForAllocation();
  if (message_arena == nullptr) {
    delete head_;
  }
  if (head) {
    ::PROTOBUF_NAMESPACE_ID::Arena* submessage_arena =
        ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper<::opentera::protobuf::TeraModuleMessage_Header>::GetOwningArena(head);
    if (message_arena != submessage_arena) {
      head = ::PROTOBUF_NAMESPACE_ID::internal::GetOwnedMessage(
          message_arena, head, submessage_arena);
    }
    
  } else {
    
  }
  head_ = head;
  // @@protoc_insertion_point(field_set_allocated:opentera.protobuf.TeraModuleMessage.head)
}

// repeated .google.protobuf.Any data = 2;
inline int TeraModuleMessage::_internal_data_size() const {
  return data_.size();
}
inline int TeraModuleMessage::data_size() const {
  return _internal_data_size();
}
inline PROTOBUF_NAMESPACE_ID::Any* TeraModuleMessage::mutable_data(int index) {
  // @@protoc_insertion_point(field_mutable:opentera.protobuf.TeraModuleMessage.data)
  return data_.Mutable(index);
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< PROTOBUF_NAMESPACE_ID::Any >*
TeraModuleMessage::mutable_data() {
  // @@protoc_insertion_point(field_mutable_list:opentera.protobuf.TeraModuleMessage.data)
  return &data_;
}
inline const PROTOBUF_NAMESPACE_ID::Any& TeraModuleMessage::_internal_data(int index) const {
  return data_.Get(index);
}
inline const PROTOBUF_NAMESPACE_ID::Any& TeraModuleMessage::data(int index) const {
  // @@protoc_insertion_point(field_get:opentera.protobuf.TeraModuleMessage.data)
  return _internal_data(index);
}
inline PROTOBUF_NAMESPACE_ID::Any* TeraModuleMessage::_internal_add_data() {
  return data_.Add();
}
inline PROTOBUF_NAMESPACE_ID::Any* TeraModuleMessage::add_data() {
  PROTOBUF_NAMESPACE_ID::Any* _add = _internal_add_data();
  // @@protoc_insertion_point(field_add:opentera.protobuf.TeraModuleMessage.data)
  return _add;
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< PROTOBUF_NAMESPACE_ID::Any >&
TeraModuleMessage::data() const {
  // @@protoc_insertion_point(field_list:opentera.protobuf.TeraModuleMessage.data)
  return data_;
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace protobuf
}  // namespace opentera

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_TeraModuleMessage_2eproto
