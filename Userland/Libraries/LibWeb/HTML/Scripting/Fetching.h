/*
 * Copyright (c) 2022-2023, networkException <networkexception@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/Fetch/Infrastructure/FetchAlgorithms.h>
#include <LibWeb/Fetch/Infrastructure/HTTP/Requests.h>
#include <LibWeb/HTML/CORSSettingAttribute.h>
#include <LibWeb/HTML/Scripting/ImportMap.h>
#include <LibWeb/HTML/Scripting/ModuleMap.h>
#include <LibWeb/HTML/Scripting/ModuleScript.h>
#include <LibWeb/ReferrerPolicy/ReferrerPolicy.h>

namespace Web::HTML {

enum class IsTopLevel {
    No,
    Yes,
};

using OnFetchScriptComplete = JS::NonnullGCPtr<JS::HeapFunction<void(JS::GCPtr<Script>)>>;
using PerformTheFetchHook = JS::GCPtr<JS::HeapFunction<WebIDL::ExceptionOr<void>(JS::NonnullGCPtr<Fetch::Infrastructure::Request>, IsTopLevel, Fetch::Infrastructure::FetchAlgorithms::ProcessResponseConsumeBodyFunction)>>;

OnFetchScriptComplete create_on_fetch_script_complete(JS::Heap& heap, Function<void(JS::GCPtr<Script>)> function);
PerformTheFetchHook create_perform_the_fetch_hook(JS::Heap& heap, Function<WebIDL::ExceptionOr<void>(JS::NonnullGCPtr<Fetch::Infrastructure::Request>, IsTopLevel, Fetch::Infrastructure::FetchAlgorithms::ProcessResponseConsumeBodyFunction)> function);

// https://html.spec.whatwg.org/multipage/webappapis.html#script-fetch-options
struct ScriptFetchOptions {
    // https://html.spec.whatwg.org/multipage/webappapis.html#concept-script-fetch-options-nonce
    String cryptographic_nonce {};

    // https://html.spec.whatwg.org/multipage/webappapis.html#concept-script-fetch-options-integrity
    String integrity_metadata {};

    // https://html.spec.whatwg.org/multipage/webappapis.html#concept-script-fetch-options-parser
    Fetch::Infrastructure::Request::ParserMetadata parser_metadata { Fetch::Infrastructure::Request::ParserMetadata::NotParserInserted };

    // https://html.spec.whatwg.org/multipage/webappapis.html#concept-script-fetch-options-credentials
    Fetch::Infrastructure::Request::CredentialsMode credentials_mode { Fetch::Infrastructure::Request::CredentialsMode::SameOrigin };

    // https://html.spec.whatwg.org/multipage/webappapis.html#concept-script-fetch-options-referrer-policy
    Optional<ReferrerPolicy::ReferrerPolicy> referrer_policy;

    // https://html.spec.whatwg.org/multipage/webappapis.html#concept-script-fetch-options-render-blocking
    bool render_blocking { false };

    // https://html.spec.whatwg.org/multipage/webappapis.html#concept-script-fetch-options-fetch-priority
    Fetch::Infrastructure::Request::Priority fetch_priority {};
};

// https://html.spec.whatwg.org/multipage/webappapis.html#default-classic-script-fetch-options
ScriptFetchOptions default_classic_script_fetch_options();

struct FetchContext : JS::GraphLoadingState::HostDefined {
    FetchContext(JS::GCPtr<JS::Value> parse_error, Fetch::Infrastructure::Request::Destination destination, JS::GCPtr<JS::Promise> perform_fetch, EnvironmentSettingsObject& fetch_client)
        : parse_error(parse_error)
        , destination(destination)
        , perform_fetch(perform_fetch)
        , fetch_client(fetch_client)
    {
    }

    JS::GCPtr<JS::Value> parse_error;                        // [[ParseError]]
    Fetch::Infrastructure::Request::Destination destination; // [[Destination]]
    JS::GCPtr<JS::Promise> perform_fetch;                    // [[PerformFetch]]
    EnvironmentSettingsObject& fetch_client;                 // [[FetchClient]]
};

DeprecatedString module_type_from_module_request(JS::ModuleRequest const&);
WebIDL::ExceptionOr<AK::URL> resolve_module_specifier(Optional<Script&> referring_script, DeprecatedString const& specifier);
WebIDL::ExceptionOr<Optional<AK::URL>> resolve_imports_match(DeprecatedString const& normalized_specifier, Optional<AK::URL> as_url, ModuleSpecifierMap const&);
Optional<AK::URL> resolve_url_like_module_specifier(DeprecatedString const& specifier, AK::URL const& base_url);

WebIDL::ExceptionOr<void> fetch_classic_script(JS::NonnullGCPtr<HTMLScriptElement>, AK::URL const&, EnvironmentSettingsObject& settings_object, ScriptFetchOptions options, CORSSettingAttribute cors_setting, String character_encoding, OnFetchScriptComplete on_complete);
WebIDL::ExceptionOr<void> fetch_classic_worker_script(AK::URL const&, EnvironmentSettingsObject& fetch_client, Fetch::Infrastructure::Request::Destination, EnvironmentSettingsObject& settings_object, PerformTheFetchHook, OnFetchScriptComplete);
void fetch_internal_module_script_graph(JS::Realm&, JS::ModuleRequest const& module_request, EnvironmentSettingsObject& fetch_client_settings_object, Fetch::Infrastructure::Request::Destination, ScriptFetchOptions const&, Script& referring_script, HashTable<ModuleLocationTuple> const& visited_set, OnFetchScriptComplete on_complete);
void fetch_external_module_script_graph(JS::Realm&, AK::URL const&, EnvironmentSettingsObject& settings_object, ScriptFetchOptions const&, OnFetchScriptComplete on_complete);
void fetch_inline_module_script_graph(JS::Realm&, DeprecatedString const& filename, DeprecatedString const& source_text, AK::URL const& base_url, EnvironmentSettingsObject& settings_object, OnFetchScriptComplete on_complete);
void fetch_single_imported_module_script(JS::Realm&, AK::URL const&, EnvironmentSettingsObject& fetch_client, Fetch::Infrastructure::Request::Destination, ScriptFetchOptions const&, EnvironmentSettingsObject& settings_object, Fetch::Infrastructure::Request::Referrer, JS::ModuleRequest const&, OnFetchScriptComplete on_complete);

void fetch_descendants_of_a_module_script(JS::Realm&, JavaScriptModuleScript& module_script, EnvironmentSettingsObject& fetch_client_settings_object, Fetch::Infrastructure::Request::Destination, HashTable<ModuleLocationTuple> visited_set, OnFetchScriptComplete callback);
void fetch_descendants_of_and_link_a_module_script(JS::Realm&, JavaScriptModuleScript&, EnvironmentSettingsObject&, Fetch::Infrastructure::Request::Destination, OnFetchScriptComplete on_complete);

enum class TopLevelModule {
    Yes,
    No
};

void fetch_single_module_script(JS::Realm&, AK::URL const&, EnvironmentSettingsObject& fetch_client, Fetch::Infrastructure::Request::Destination, ScriptFetchOptions const&, EnvironmentSettingsObject& settings_object, Web::Fetch::Infrastructure::Request::ReferrerType const&, Optional<JS::ModuleRequest> const&, TopLevelModule, OnFetchScriptComplete callback);

}
