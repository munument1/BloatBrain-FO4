includes("lib/commonlibf4")

set_project("BloatBrain-FO4")
set_version("0.1.0")
set_languages("c++23")
set_warnings("allextra")

add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

target("BloatBrainFO4")
    add_rules("commonlibf4.plugin", {
        name = "BloatBrainFO4",
        author = "munument1",
        description = "External neural controller bridge for Fallout 4 Bloatflies"
    })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
    add_syslinks("ws2_32")

target("ProtocolV2Tests")
    set_kind("binary")
    set_default(false)
    add_files("src/ProtocolV2.cpp", "tests/ProtocolV2Tests.cpp")
    add_includedirs("src")
    add_tests("protocol-v2")
