/*
 * Copyright (c) 2020, Matthew Olsson <mattco@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Function.h>
#include <LibJS/Heap/Heap.h>
#include <LibJS/Runtime/GlobalObject.h>
#include <LibJS/Runtime/PrimitiveString.h>
#include <LibJS/Runtime/RegExpObject.h>
#include <LibJS/Runtime/Value.h>

namespace JS {

static Flags options_from(GlobalObject& global_object, const String& flags)
{
    auto& vm = global_object.vm();
    bool d = false, g = false, i = false, m = false, s = false, u = false, y = false;
    Flags options {
        // JS regexps are all 'global' by default as per our definition, but the "global" flag enables "stateful".
        // FIXME: Enable 'BrowserExtended' only if in a browser context.
        .effective_flags = { (regex::ECMAScriptFlags)regex::AllFlags::Global | (regex::ECMAScriptFlags)regex::AllFlags::SkipTrimEmptyMatches | regex::ECMAScriptFlags::BrowserExtended },
        .declared_flags = {},
    };

    for (auto ch : flags) {
        switch (ch) {
        case 'd':
            if (d)
                vm.throw_exception<SyntaxError>(global_object, ErrorType::RegExpObjectRepeatedFlag, ch);
            d = true;
            break;
        case 'g':
            if (g)
                vm.throw_exception<SyntaxError>(global_object, ErrorType::RegExpObjectRepeatedFlag, ch);
            g = true;
            options.effective_flags |= regex::ECMAScriptFlags::Global;
            options.declared_flags |= regex::ECMAScriptFlags::Global;
            break;
        case 'i':
            if (i)
                vm.throw_exception<SyntaxError>(global_object, ErrorType::RegExpObjectRepeatedFlag, ch);
            i = true;
            options.effective_flags |= regex::ECMAScriptFlags::Insensitive;
            options.declared_flags |= regex::ECMAScriptFlags::Insensitive;
            break;
        case 'm':
            if (m)
                vm.throw_exception<SyntaxError>(global_object, ErrorType::RegExpObjectRepeatedFlag, ch);
            m = true;
            options.effective_flags |= regex::ECMAScriptFlags::Multiline;
            options.declared_flags |= regex::ECMAScriptFlags::Multiline;
            break;
        case 's':
            if (s)
                vm.throw_exception<SyntaxError>(global_object, ErrorType::RegExpObjectRepeatedFlag, ch);
            s = true;
            options.effective_flags |= regex::ECMAScriptFlags::SingleLine;
            options.declared_flags |= regex::ECMAScriptFlags::SingleLine;
            break;
        case 'u':
            if (u)
                vm.throw_exception<SyntaxError>(global_object, ErrorType::RegExpObjectRepeatedFlag, ch);
            u = true;
            options.effective_flags |= regex::ECMAScriptFlags::Unicode;
            options.declared_flags |= regex::ECMAScriptFlags::Unicode;
            break;
        case 'y':
            if (y)
                vm.throw_exception<SyntaxError>(global_object, ErrorType::RegExpObjectRepeatedFlag, ch);
            y = true;
            // Now for the more interesting flag, 'sticky' actually unsets 'global', part of which is the default.
            options.effective_flags.reset_flag(regex::ECMAScriptFlags::Global);
            // "What's the difference between sticky and global, then", that's simple.
            // all the other flags imply 'global', and the "global" flag implies 'stateful';
            // however, the "sticky" flag does *not* imply 'global', only 'stateful'.
            options.effective_flags |= (regex::ECMAScriptFlags)regex::AllFlags::Internal_Stateful;
            options.effective_flags |= regex::ECMAScriptFlags::Sticky;
            options.declared_flags |= regex::ECMAScriptFlags::Sticky;
            break;
        default:
            vm.throw_exception<SyntaxError>(global_object, ErrorType::RegExpObjectBadFlag, ch);
            return options;
        }
    }

    return options;
}

RegExpObject* RegExpObject::create(GlobalObject& global_object, String pattern, String flags)
{
    return global_object.heap().allocate<RegExpObject>(global_object, pattern, flags, *global_object.regexp_prototype());
}

RegExpObject::RegExpObject(String pattern, String flags, Object& prototype)
    : Object(prototype)
    , m_pattern(pattern)
    , m_flags(flags)
    , m_active_flags(options_from(global_object(), m_flags))
    , m_regex(pattern, m_active_flags.effective_flags)
{
    if (m_regex.parser_result.error != regex::Error::NoError) {
        vm().throw_exception<SyntaxError>(global_object(), ErrorType::RegExpCompileError, m_regex.error_string());
    }
}

RegExpObject::~RegExpObject()
{
}

void RegExpObject::initialize(GlobalObject& global_object)
{
    auto& vm = this->vm();
    Object::initialize(global_object);
    define_direct_property(vm.names.lastIndex, {}, Attribute::Writable);
}

// 22.2.3.2.4 RegExpCreate ( P, F ), https://tc39.es/ecma262/#sec-regexpcreate
RegExpObject* regexp_create(GlobalObject& global_object, Value pattern, Value flags)
{
    auto& vm = global_object.vm();
    String p;
    if (pattern.is_undefined()) {
        p = String::empty();
    } else {
        p = pattern.to_string(global_object);
        if (vm.exception())
            return {};
    }
    String f;
    if (flags.is_undefined()) {
        f = String::empty();
    } else {
        f = flags.to_string(global_object);
        if (vm.exception())
            return {};
    }
    auto* object = RegExpObject::create(global_object, move(p), move(f));
    object->set(vm.names.lastIndex, Value(0), true);
    if (vm.exception())
        return {};
    return object;
}

}
