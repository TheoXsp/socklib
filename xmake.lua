set_version("1.0.0")
add_rules("mode.debug", "mode.release")

target("socklib")
    set_languages("c++23")
    set_kind("static")
    add_files("socket/**.cpp")
    add_files("tcpSocket/**.cpp")
    add_files("udpSocket/**.cpp")
    add_includedirs(".")

    if is_plat("mingw") then
        add_ldflags("-lws2_32", "-static")
    end