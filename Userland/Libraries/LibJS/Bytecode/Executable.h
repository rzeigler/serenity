/*
 * Copyright (c) 2021-2023, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/DeprecatedFlyString.h>
#include <AK/HashMap.h>
#include <AK/NonnullOwnPtr.h>
#include <AK/OwnPtr.h>
#include <AK/WeakPtr.h>
#include <LibJS/Bytecode/IdentifierTable.h>
#include <LibJS/Bytecode/Label.h>
#include <LibJS/Bytecode/StringTable.h>
#include <LibJS/Forward.h>
#include <LibJS/Runtime/EnvironmentCoordinate.h>

namespace JS::JIT {
class NativeExecutable;
}

namespace JS::Bytecode {

struct PropertyLookupCache {
    static FlatPtr shape_offset() { return OFFSET_OF(PropertyLookupCache, shape); }
    static FlatPtr property_offset_offset() { return OFFSET_OF(PropertyLookupCache, property_offset); }
    static FlatPtr unique_shape_serial_number_offset() { return OFFSET_OF(PropertyLookupCache, unique_shape_serial_number); }

    WeakPtr<Shape> shape;
    Optional<u32> property_offset;
    u64 unique_shape_serial_number { 0 };
};

struct GlobalVariableCache : public PropertyLookupCache {
    static FlatPtr environment_serial_number_offset() { return OFFSET_OF(GlobalVariableCache, environment_serial_number); }

    u64 environment_serial_number { 0 };
};

using EnvironmentVariableCache = Optional<EnvironmentCoordinate>;

struct SourceRecord {
    u32 source_start_offset {};
    u32 source_end_offset {};
};

class Executable final : public RefCounted<Executable> {
public:
    Executable(
        NonnullOwnPtr<IdentifierTable>,
        NonnullOwnPtr<StringTable>,
        NonnullOwnPtr<RegexTable>,
        NonnullRefPtr<SourceCode const>,
        size_t number_of_property_lookup_caches,
        size_t number_of_global_variable_caches,
        size_t number_of_environment_variable_caches,
        size_t number_of_registers,
        Vector<NonnullOwnPtr<BasicBlock>>,
        bool is_strict_mode);

    ~Executable();

    DeprecatedFlyString name;
    Vector<PropertyLookupCache> property_lookup_caches;
    Vector<GlobalVariableCache> global_variable_caches;
    Vector<EnvironmentVariableCache> environment_variable_caches;
    Vector<NonnullOwnPtr<BasicBlock>> basic_blocks;
    NonnullOwnPtr<StringTable> string_table;
    NonnullOwnPtr<IdentifierTable> identifier_table;
    NonnullOwnPtr<RegexTable> regex_table;

    NonnullRefPtr<SourceCode const> source_code;
    size_t number_of_registers { 0 };
    bool is_strict_mode { false };

    DeprecatedString const& get_string(StringTableIndex index) const { return string_table->get(index); }
    DeprecatedFlyString const& get_identifier(IdentifierTableIndex index) const { return identifier_table->get(index); }

    void dump() const;

    JIT::NativeExecutable const* get_or_create_native_executable();
    JIT::NativeExecutable const* native_executable() const { return m_native_executable; }

private:
    OwnPtr<JIT::NativeExecutable> m_native_executable;
    bool m_did_try_jitting { false };
};

}
