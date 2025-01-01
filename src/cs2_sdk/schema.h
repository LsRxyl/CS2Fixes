/**
 * =============================================================================
 * CS2Fixes
 * Copyright (C) 2023-2025 Source2ZE
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifdef _WIN32
	#pragma warning(push)
	#pragma warning(disable : 4005)
#endif

#include <type_traits>

#ifdef _WIN32
	#pragma warning(pop)
#endif

#include "../addresses.h"
#include "const.h"
#include "stdint.h"
#include "tier0/dbg.h"
#include "virtual.h"
#undef schema

struct SchemaKey
{
	int32 offset;
	bool networked;
};

void EntityNetworkStateChanged(uintptr_t entityInstance, uint nOffset);
void ChainNetworkStateChanged(uintptr_t networkVarChainer, uint nLocalOffset);

constexpr uint32_t val_32_const = 0x811c9dc5;
constexpr uint32_t prime_32_const = 0x1000193;
constexpr uint64_t val_64_const = 0xcbf29ce484222325;
constexpr uint64_t prime_64_const = 0x100000001b3;

inline constexpr uint32_t hash_32_fnv1a_const(const char* const str, const uint32_t value = val_32_const) noexcept
{
	return (str[0] == '\0') ? value : hash_32_fnv1a_const(&str[1], (value ^ uint32_t(str[0])) * prime_32_const);
}

inline constexpr uint64_t hash_64_fnv1a_const(const char* const str, const uint64_t value = val_64_const) noexcept
{
	return (str[0] == '\0') ? value : hash_64_fnv1a_const(&str[1], (value ^ uint64_t(str[0])) * prime_64_const);
}

#define SCHEMA_FIELD_OFFSET(type, varName, extra_offset)                                                              \
	class varName##_prop                                                                                              \
	{                                                                                                                 \
	public:                                                                                                           \
		std::add_lvalue_reference_t<type> Get()                                                                       \
		{                                                                                                             \
			static constexpr auto datatable_hash = hash_32_fnv1a_const(ThisClassName);                                \
			static constexpr auto prop_hash = hash_32_fnv1a_const(#varName);                                          \
                                                                                                                      \
			static const auto m_key =                                                                                 \
				schema::GetOffset(ThisClassName, datatable_hash, #varName, prop_hash);                                \
                                                                                                                      \
			static const size_t offset = offsetof(ThisClass, varName);                                                \
			ThisClass* pThisClass = (ThisClass*)((byte*)this - offset);                                               \
                                                                                                                      \
			return *reinterpret_cast<std::add_pointer_t<type>>(                                                       \
				(uintptr_t)(pThisClass) + m_key.offset + extra_offset);                                               \
		}                                                                                                             \
		void Set(type val)                                                                                            \
		{                                                                                                             \
			static constexpr auto datatable_hash = hash_32_fnv1a_const(ThisClassName);                                \
			static constexpr auto prop_hash = hash_32_fnv1a_const(#varName);                                          \
                                                                                                                      \
			static const auto m_key =                                                                                 \
				schema::GetOffset(ThisClassName, datatable_hash, #varName, prop_hash);                                \
                                                                                                                      \
			static const auto m_chain =                                                                               \
				schema::FindChainOffset(ThisClassName);                                                               \
                                                                                                                      \
			static const size_t offset = offsetof(ThisClass, varName);                                                \
			ThisClass* pThisClass = (ThisClass*)((byte*)this - offset);                                               \
                                                                                                                      \
			if (m_chain != 0 && m_key.networked)                                                                      \
			{                                                                                                         \
				DevMsg("Found chain offset %d for %s::%s\n", m_chain, ThisClassName, #varName);                       \
				ChainNetworkStateChanged((uintptr_t)(pThisClass) + m_chain, m_key.offset + extra_offset);             \
			}                                                                                                         \
			else if (m_key.networked)                                                                                 \
			{                                                                                                         \
				/* WIP: Works fine for most props, but inlined classes in the middle of a class will                  \
					need to have their this pointer corrected by the offset .*/                                       \
				if (!IsStruct)                                                                                        \
					EntityNetworkStateChanged((uintptr_t)pThisClass, m_key.offset + extra_offset);                    \
				else                                                                                                  \
					CALL_VIRTUAL(void, 1, pThisClass, m_key.offset + extra_offset, 0xFFFFFFFF, 0xFFFFFFFF);           \
			}                                                                                                         \
			*reinterpret_cast<std::add_pointer_t<type>>((uintptr_t)(pThisClass) + m_key.offset + extra_offset) = val; \
		}                                                                                                             \
		operator std::add_lvalue_reference_t<type>()                                                                  \
		{                                                                                                             \
			return Get();                                                                                             \
		}                                                                                                             \
		std::add_lvalue_reference_t<type> operator()()                                                                \
		{                                                                                                             \
			return Get();                                                                                             \
		}                                                                                                             \
		std::add_lvalue_reference_t<type> operator->()                                                                \
		{                                                                                                             \
			return Get();                                                                                             \
		}                                                                                                             \
		void operator()(type val)                                                                                     \
		{                                                                                                             \
			Set(val);                                                                                                 \
		}                                                                                                             \
		void operator=(type val)                                                                                      \
		{                                                                                                             \
			Set(val);                                                                                                 \
		}                                                                                                             \
	} varName;

#define SCHEMA_FIELD_POINTER_OFFSET(type, varName, extra_offset)                       \
	class varName##_prop                                                               \
	{                                                                                  \
	public:                                                                            \
		type* Get()                                                                    \
		{                                                                              \
			static constexpr auto datatable_hash = hash_32_fnv1a_const(ThisClassName); \
			static constexpr auto prop_hash = hash_32_fnv1a_const(#varName);           \
                                                                                       \
			static const auto m_key =                                                  \
				schema::GetOffset(ThisClassName, datatable_hash, #varName, prop_hash); \
                                                                                       \
			static const size_t offset = offsetof(ThisClass, varName);                 \
			ThisClass* pThisClass = (ThisClass*)((byte*)this - offset);                \
                                                                                       \
			return reinterpret_cast<std::add_pointer_t<type>>(                         \
				(uintptr_t)(pThisClass) + m_key.offset + extra_offset);                \
		}                                                                              \
		operator type*()                                                               \
		{                                                                              \
			return Get();                                                              \
		}                                                                              \
		type* operator()()                                                             \
		{                                                                              \
			return Get();                                                              \
		}                                                                              \
		type* operator->()                                                             \
		{                                                                              \
			return Get();                                                              \
		}                                                                              \
	} varName;

// Use this when you want the member's value itself
#define SCHEMA_FIELD(type, varName) \
	SCHEMA_FIELD_OFFSET(type, varName, 0)

// Use this when you want a pointer to a member
#define SCHEMA_FIELD_POINTER(type, varName) \
	SCHEMA_FIELD_POINTER_OFFSET(type, varName, 0)

namespace schema
{
	int16_t FindChainOffset(const char* className);
	SchemaKey GetOffset(const char* className, uint32_t classKey, const char* memberName, uint32_t memberKey);
} // namespace schema

#define DECLARE_SCHEMA_CLASS_BASE(className, isStruct)       \
	typedef className ThisClass;                             \
	static constexpr const char* ThisClassName = #className; \
	static constexpr bool IsStruct = isStruct;

#define DECLARE_SCHEMA_CLASS(className) DECLARE_SCHEMA_CLASS_BASE(className, false)

// Use this for classes that can be wholly included within other classes (like CCollisionProperty within CBaseModelEntity)
#define DECLARE_SCHEMA_CLASS_INLINE(className) \
	DECLARE_SCHEMA_CLASS_BASE(className, true)