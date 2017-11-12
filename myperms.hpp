//
// Created by koncord on 12.11.17.
//

// copied from experimental/bits/fs_fwd.h

#pragma once

#include <type_traits>

namespace myfs
{

    enum class perms : unsigned
    {
        none = 0,
        owner_read = 0400,
        owner_write = 0200,
        owner_exec = 0100,
        owner_all = 0700,
        group_read = 040,
        group_write = 020,
        group_exec = 010,
        group_all = 070,
        others_read = 04,
        others_write = 02,
        others_exec = 01,
        others_all = 07,
        all = 0777,
        set_uid = 04000,
        set_gid = 02000,
        sticky_bit = 01000,
        mask = 07777,
        unknown = 0xFFFF,
        add_perms = 0x10000,
        remove_perms = 0x20000,
        symlink_nofollow = 0x40000
    };

    constexpr perms
    operator&(perms __x, perms __y) noexcept
    {
        using __utype = typename std::underlying_type<perms>::type;
        return static_cast<perms>(
                static_cast<__utype>(__x) & static_cast<__utype>(__y));
    }

    constexpr perms
    operator|(perms __x, perms __y) noexcept
    {
        using __utype = typename std::underlying_type<perms>::type;
        return static_cast<perms>(
                static_cast<__utype>(__x) | static_cast<__utype>(__y));
    }

    constexpr perms
    operator^(perms __x, perms __y) noexcept
    {
        using __utype = typename std::underlying_type<perms>::type;
        return static_cast<perms>(
                static_cast<__utype>(__x) ^ static_cast<__utype>(__y));
    }

    constexpr perms
    operator~(perms __x) noexcept
    {
        using __utype = typename std::underlying_type<perms>::type;
        return static_cast<perms>(~static_cast<__utype>(__x));
    }

    inline perms &
    operator&=(perms &__x, perms __y) noexcept
    {
        return __x = __x & __y;
    }

    inline perms &
    operator|=(perms &__x, perms __y) noexcept
    {
        return __x = __x | __y;
    }

    inline perms &
    operator^=(perms &__x, perms __y) noexcept
    {
        return __x = __x ^ __y;
    }
}