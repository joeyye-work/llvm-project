; RUN: llc -mtriple=groq-unknown-unknown -mcpu=tsp-v1 -verify-machineinstrs -misched=ilpmax < %s | FileCheck %s

define void @queue_pool_144_add_sub(i32 %x, i32 %y) {
; CHECK-LABEL: queue_pool_144_add_sub:
; Current scaffold lowers both add-immediate and sub-immediate patterns through
; add-style instructions.
; CHECK-COUNT-288: add
  %a1 = add i32 %x, 1
  %s1 = sub i32 %y, 1
  call void asm sideeffect "", "r,r"(i32 %a1, i32 %s1)
  %a2 = add i32 %x, 2
  %s2 = sub i32 %y, 2
  call void asm sideeffect "", "r,r"(i32 %a2, i32 %s2)
  %a3 = add i32 %x, 3
  %s3 = sub i32 %y, 3
  call void asm sideeffect "", "r,r"(i32 %a3, i32 %s3)
  %a4 = add i32 %x, 4
  %s4 = sub i32 %y, 4
  call void asm sideeffect "", "r,r"(i32 %a4, i32 %s4)
  %a5 = add i32 %x, 5
  %s5 = sub i32 %y, 5
  call void asm sideeffect "", "r,r"(i32 %a5, i32 %s5)
  %a6 = add i32 %x, 6
  %s6 = sub i32 %y, 6
  call void asm sideeffect "", "r,r"(i32 %a6, i32 %s6)
  %a7 = add i32 %x, 7
  %s7 = sub i32 %y, 7
  call void asm sideeffect "", "r,r"(i32 %a7, i32 %s7)
  %a8 = add i32 %x, 8
  %s8 = sub i32 %y, 8
  call void asm sideeffect "", "r,r"(i32 %a8, i32 %s8)
  %a9 = add i32 %x, 9
  %s9 = sub i32 %y, 9
  call void asm sideeffect "", "r,r"(i32 %a9, i32 %s9)
  %a10 = add i32 %x, 10
  %s10 = sub i32 %y, 10
  call void asm sideeffect "", "r,r"(i32 %a10, i32 %s10)
  %a11 = add i32 %x, 11
  %s11 = sub i32 %y, 11
  call void asm sideeffect "", "r,r"(i32 %a11, i32 %s11)
  %a12 = add i32 %x, 12
  %s12 = sub i32 %y, 12
  call void asm sideeffect "", "r,r"(i32 %a12, i32 %s12)
  %a13 = add i32 %x, 13
  %s13 = sub i32 %y, 13
  call void asm sideeffect "", "r,r"(i32 %a13, i32 %s13)
  %a14 = add i32 %x, 14
  %s14 = sub i32 %y, 14
  call void asm sideeffect "", "r,r"(i32 %a14, i32 %s14)
  %a15 = add i32 %x, 15
  %s15 = sub i32 %y, 15
  call void asm sideeffect "", "r,r"(i32 %a15, i32 %s15)
  %a16 = add i32 %x, 16
  %s16 = sub i32 %y, 16
  call void asm sideeffect "", "r,r"(i32 %a16, i32 %s16)
  %a17 = add i32 %x, 17
  %s17 = sub i32 %y, 17
  call void asm sideeffect "", "r,r"(i32 %a17, i32 %s17)
  %a18 = add i32 %x, 18
  %s18 = sub i32 %y, 18
  call void asm sideeffect "", "r,r"(i32 %a18, i32 %s18)
  %a19 = add i32 %x, 19
  %s19 = sub i32 %y, 19
  call void asm sideeffect "", "r,r"(i32 %a19, i32 %s19)
  %a20 = add i32 %x, 20
  %s20 = sub i32 %y, 20
  call void asm sideeffect "", "r,r"(i32 %a20, i32 %s20)
  %a21 = add i32 %x, 21
  %s21 = sub i32 %y, 21
  call void asm sideeffect "", "r,r"(i32 %a21, i32 %s21)
  %a22 = add i32 %x, 22
  %s22 = sub i32 %y, 22
  call void asm sideeffect "", "r,r"(i32 %a22, i32 %s22)
  %a23 = add i32 %x, 23
  %s23 = sub i32 %y, 23
  call void asm sideeffect "", "r,r"(i32 %a23, i32 %s23)
  %a24 = add i32 %x, 24
  %s24 = sub i32 %y, 24
  call void asm sideeffect "", "r,r"(i32 %a24, i32 %s24)
  %a25 = add i32 %x, 25
  %s25 = sub i32 %y, 25
  call void asm sideeffect "", "r,r"(i32 %a25, i32 %s25)
  %a26 = add i32 %x, 26
  %s26 = sub i32 %y, 26
  call void asm sideeffect "", "r,r"(i32 %a26, i32 %s26)
  %a27 = add i32 %x, 27
  %s27 = sub i32 %y, 27
  call void asm sideeffect "", "r,r"(i32 %a27, i32 %s27)
  %a28 = add i32 %x, 28
  %s28 = sub i32 %y, 28
  call void asm sideeffect "", "r,r"(i32 %a28, i32 %s28)
  %a29 = add i32 %x, 29
  %s29 = sub i32 %y, 29
  call void asm sideeffect "", "r,r"(i32 %a29, i32 %s29)
  %a30 = add i32 %x, 30
  %s30 = sub i32 %y, 30
  call void asm sideeffect "", "r,r"(i32 %a30, i32 %s30)
  %a31 = add i32 %x, 31
  %s31 = sub i32 %y, 31
  call void asm sideeffect "", "r,r"(i32 %a31, i32 %s31)
  %a32 = add i32 %x, 32
  %s32 = sub i32 %y, 32
  call void asm sideeffect "", "r,r"(i32 %a32, i32 %s32)
  %a33 = add i32 %x, 33
  %s33 = sub i32 %y, 33
  call void asm sideeffect "", "r,r"(i32 %a33, i32 %s33)
  %a34 = add i32 %x, 34
  %s34 = sub i32 %y, 34
  call void asm sideeffect "", "r,r"(i32 %a34, i32 %s34)
  %a35 = add i32 %x, 35
  %s35 = sub i32 %y, 35
  call void asm sideeffect "", "r,r"(i32 %a35, i32 %s35)
  %a36 = add i32 %x, 36
  %s36 = sub i32 %y, 36
  call void asm sideeffect "", "r,r"(i32 %a36, i32 %s36)
  %a37 = add i32 %x, 37
  %s37 = sub i32 %y, 37
  call void asm sideeffect "", "r,r"(i32 %a37, i32 %s37)
  %a38 = add i32 %x, 38
  %s38 = sub i32 %y, 38
  call void asm sideeffect "", "r,r"(i32 %a38, i32 %s38)
  %a39 = add i32 %x, 39
  %s39 = sub i32 %y, 39
  call void asm sideeffect "", "r,r"(i32 %a39, i32 %s39)
  %a40 = add i32 %x, 40
  %s40 = sub i32 %y, 40
  call void asm sideeffect "", "r,r"(i32 %a40, i32 %s40)
  %a41 = add i32 %x, 41
  %s41 = sub i32 %y, 41
  call void asm sideeffect "", "r,r"(i32 %a41, i32 %s41)
  %a42 = add i32 %x, 42
  %s42 = sub i32 %y, 42
  call void asm sideeffect "", "r,r"(i32 %a42, i32 %s42)
  %a43 = add i32 %x, 43
  %s43 = sub i32 %y, 43
  call void asm sideeffect "", "r,r"(i32 %a43, i32 %s43)
  %a44 = add i32 %x, 44
  %s44 = sub i32 %y, 44
  call void asm sideeffect "", "r,r"(i32 %a44, i32 %s44)
  %a45 = add i32 %x, 45
  %s45 = sub i32 %y, 45
  call void asm sideeffect "", "r,r"(i32 %a45, i32 %s45)
  %a46 = add i32 %x, 46
  %s46 = sub i32 %y, 46
  call void asm sideeffect "", "r,r"(i32 %a46, i32 %s46)
  %a47 = add i32 %x, 47
  %s47 = sub i32 %y, 47
  call void asm sideeffect "", "r,r"(i32 %a47, i32 %s47)
  %a48 = add i32 %x, 48
  %s48 = sub i32 %y, 48
  call void asm sideeffect "", "r,r"(i32 %a48, i32 %s48)
  %a49 = add i32 %x, 49
  %s49 = sub i32 %y, 49
  call void asm sideeffect "", "r,r"(i32 %a49, i32 %s49)
  %a50 = add i32 %x, 50
  %s50 = sub i32 %y, 50
  call void asm sideeffect "", "r,r"(i32 %a50, i32 %s50)
  %a51 = add i32 %x, 51
  %s51 = sub i32 %y, 51
  call void asm sideeffect "", "r,r"(i32 %a51, i32 %s51)
  %a52 = add i32 %x, 52
  %s52 = sub i32 %y, 52
  call void asm sideeffect "", "r,r"(i32 %a52, i32 %s52)
  %a53 = add i32 %x, 53
  %s53 = sub i32 %y, 53
  call void asm sideeffect "", "r,r"(i32 %a53, i32 %s53)
  %a54 = add i32 %x, 54
  %s54 = sub i32 %y, 54
  call void asm sideeffect "", "r,r"(i32 %a54, i32 %s54)
  %a55 = add i32 %x, 55
  %s55 = sub i32 %y, 55
  call void asm sideeffect "", "r,r"(i32 %a55, i32 %s55)
  %a56 = add i32 %x, 56
  %s56 = sub i32 %y, 56
  call void asm sideeffect "", "r,r"(i32 %a56, i32 %s56)
  %a57 = add i32 %x, 57
  %s57 = sub i32 %y, 57
  call void asm sideeffect "", "r,r"(i32 %a57, i32 %s57)
  %a58 = add i32 %x, 58
  %s58 = sub i32 %y, 58
  call void asm sideeffect "", "r,r"(i32 %a58, i32 %s58)
  %a59 = add i32 %x, 59
  %s59 = sub i32 %y, 59
  call void asm sideeffect "", "r,r"(i32 %a59, i32 %s59)
  %a60 = add i32 %x, 60
  %s60 = sub i32 %y, 60
  call void asm sideeffect "", "r,r"(i32 %a60, i32 %s60)
  %a61 = add i32 %x, 61
  %s61 = sub i32 %y, 61
  call void asm sideeffect "", "r,r"(i32 %a61, i32 %s61)
  %a62 = add i32 %x, 62
  %s62 = sub i32 %y, 62
  call void asm sideeffect "", "r,r"(i32 %a62, i32 %s62)
  %a63 = add i32 %x, 63
  %s63 = sub i32 %y, 63
  call void asm sideeffect "", "r,r"(i32 %a63, i32 %s63)
  %a64 = add i32 %x, 64
  %s64 = sub i32 %y, 64
  call void asm sideeffect "", "r,r"(i32 %a64, i32 %s64)
  %a65 = add i32 %x, 65
  %s65 = sub i32 %y, 65
  call void asm sideeffect "", "r,r"(i32 %a65, i32 %s65)
  %a66 = add i32 %x, 66
  %s66 = sub i32 %y, 66
  call void asm sideeffect "", "r,r"(i32 %a66, i32 %s66)
  %a67 = add i32 %x, 67
  %s67 = sub i32 %y, 67
  call void asm sideeffect "", "r,r"(i32 %a67, i32 %s67)
  %a68 = add i32 %x, 68
  %s68 = sub i32 %y, 68
  call void asm sideeffect "", "r,r"(i32 %a68, i32 %s68)
  %a69 = add i32 %x, 69
  %s69 = sub i32 %y, 69
  call void asm sideeffect "", "r,r"(i32 %a69, i32 %s69)
  %a70 = add i32 %x, 70
  %s70 = sub i32 %y, 70
  call void asm sideeffect "", "r,r"(i32 %a70, i32 %s70)
  %a71 = add i32 %x, 71
  %s71 = sub i32 %y, 71
  call void asm sideeffect "", "r,r"(i32 %a71, i32 %s71)
  %a72 = add i32 %x, 72
  %s72 = sub i32 %y, 72
  call void asm sideeffect "", "r,r"(i32 %a72, i32 %s72)
  %a73 = add i32 %x, 73
  %s73 = sub i32 %y, 73
  call void asm sideeffect "", "r,r"(i32 %a73, i32 %s73)
  %a74 = add i32 %x, 74
  %s74 = sub i32 %y, 74
  call void asm sideeffect "", "r,r"(i32 %a74, i32 %s74)
  %a75 = add i32 %x, 75
  %s75 = sub i32 %y, 75
  call void asm sideeffect "", "r,r"(i32 %a75, i32 %s75)
  %a76 = add i32 %x, 76
  %s76 = sub i32 %y, 76
  call void asm sideeffect "", "r,r"(i32 %a76, i32 %s76)
  %a77 = add i32 %x, 77
  %s77 = sub i32 %y, 77
  call void asm sideeffect "", "r,r"(i32 %a77, i32 %s77)
  %a78 = add i32 %x, 78
  %s78 = sub i32 %y, 78
  call void asm sideeffect "", "r,r"(i32 %a78, i32 %s78)
  %a79 = add i32 %x, 79
  %s79 = sub i32 %y, 79
  call void asm sideeffect "", "r,r"(i32 %a79, i32 %s79)
  %a80 = add i32 %x, 80
  %s80 = sub i32 %y, 80
  call void asm sideeffect "", "r,r"(i32 %a80, i32 %s80)
  %a81 = add i32 %x, 81
  %s81 = sub i32 %y, 81
  call void asm sideeffect "", "r,r"(i32 %a81, i32 %s81)
  %a82 = add i32 %x, 82
  %s82 = sub i32 %y, 82
  call void asm sideeffect "", "r,r"(i32 %a82, i32 %s82)
  %a83 = add i32 %x, 83
  %s83 = sub i32 %y, 83
  call void asm sideeffect "", "r,r"(i32 %a83, i32 %s83)
  %a84 = add i32 %x, 84
  %s84 = sub i32 %y, 84
  call void asm sideeffect "", "r,r"(i32 %a84, i32 %s84)
  %a85 = add i32 %x, 85
  %s85 = sub i32 %y, 85
  call void asm sideeffect "", "r,r"(i32 %a85, i32 %s85)
  %a86 = add i32 %x, 86
  %s86 = sub i32 %y, 86
  call void asm sideeffect "", "r,r"(i32 %a86, i32 %s86)
  %a87 = add i32 %x, 87
  %s87 = sub i32 %y, 87
  call void asm sideeffect "", "r,r"(i32 %a87, i32 %s87)
  %a88 = add i32 %x, 88
  %s88 = sub i32 %y, 88
  call void asm sideeffect "", "r,r"(i32 %a88, i32 %s88)
  %a89 = add i32 %x, 89
  %s89 = sub i32 %y, 89
  call void asm sideeffect "", "r,r"(i32 %a89, i32 %s89)
  %a90 = add i32 %x, 90
  %s90 = sub i32 %y, 90
  call void asm sideeffect "", "r,r"(i32 %a90, i32 %s90)
  %a91 = add i32 %x, 91
  %s91 = sub i32 %y, 91
  call void asm sideeffect "", "r,r"(i32 %a91, i32 %s91)
  %a92 = add i32 %x, 92
  %s92 = sub i32 %y, 92
  call void asm sideeffect "", "r,r"(i32 %a92, i32 %s92)
  %a93 = add i32 %x, 93
  %s93 = sub i32 %y, 93
  call void asm sideeffect "", "r,r"(i32 %a93, i32 %s93)
  %a94 = add i32 %x, 94
  %s94 = sub i32 %y, 94
  call void asm sideeffect "", "r,r"(i32 %a94, i32 %s94)
  %a95 = add i32 %x, 95
  %s95 = sub i32 %y, 95
  call void asm sideeffect "", "r,r"(i32 %a95, i32 %s95)
  %a96 = add i32 %x, 96
  %s96 = sub i32 %y, 96
  call void asm sideeffect "", "r,r"(i32 %a96, i32 %s96)
  %a97 = add i32 %x, 97
  %s97 = sub i32 %y, 97
  call void asm sideeffect "", "r,r"(i32 %a97, i32 %s97)
  %a98 = add i32 %x, 98
  %s98 = sub i32 %y, 98
  call void asm sideeffect "", "r,r"(i32 %a98, i32 %s98)
  %a99 = add i32 %x, 99
  %s99 = sub i32 %y, 99
  call void asm sideeffect "", "r,r"(i32 %a99, i32 %s99)
  %a100 = add i32 %x, 100
  %s100 = sub i32 %y, 100
  call void asm sideeffect "", "r,r"(i32 %a100, i32 %s100)
  %a101 = add i32 %x, 101
  %s101 = sub i32 %y, 101
  call void asm sideeffect "", "r,r"(i32 %a101, i32 %s101)
  %a102 = add i32 %x, 102
  %s102 = sub i32 %y, 102
  call void asm sideeffect "", "r,r"(i32 %a102, i32 %s102)
  %a103 = add i32 %x, 103
  %s103 = sub i32 %y, 103
  call void asm sideeffect "", "r,r"(i32 %a103, i32 %s103)
  %a104 = add i32 %x, 104
  %s104 = sub i32 %y, 104
  call void asm sideeffect "", "r,r"(i32 %a104, i32 %s104)
  %a105 = add i32 %x, 105
  %s105 = sub i32 %y, 105
  call void asm sideeffect "", "r,r"(i32 %a105, i32 %s105)
  %a106 = add i32 %x, 106
  %s106 = sub i32 %y, 106
  call void asm sideeffect "", "r,r"(i32 %a106, i32 %s106)
  %a107 = add i32 %x, 107
  %s107 = sub i32 %y, 107
  call void asm sideeffect "", "r,r"(i32 %a107, i32 %s107)
  %a108 = add i32 %x, 108
  %s108 = sub i32 %y, 108
  call void asm sideeffect "", "r,r"(i32 %a108, i32 %s108)
  %a109 = add i32 %x, 109
  %s109 = sub i32 %y, 109
  call void asm sideeffect "", "r,r"(i32 %a109, i32 %s109)
  %a110 = add i32 %x, 110
  %s110 = sub i32 %y, 110
  call void asm sideeffect "", "r,r"(i32 %a110, i32 %s110)
  %a111 = add i32 %x, 111
  %s111 = sub i32 %y, 111
  call void asm sideeffect "", "r,r"(i32 %a111, i32 %s111)
  %a112 = add i32 %x, 112
  %s112 = sub i32 %y, 112
  call void asm sideeffect "", "r,r"(i32 %a112, i32 %s112)
  %a113 = add i32 %x, 113
  %s113 = sub i32 %y, 113
  call void asm sideeffect "", "r,r"(i32 %a113, i32 %s113)
  %a114 = add i32 %x, 114
  %s114 = sub i32 %y, 114
  call void asm sideeffect "", "r,r"(i32 %a114, i32 %s114)
  %a115 = add i32 %x, 115
  %s115 = sub i32 %y, 115
  call void asm sideeffect "", "r,r"(i32 %a115, i32 %s115)
  %a116 = add i32 %x, 116
  %s116 = sub i32 %y, 116
  call void asm sideeffect "", "r,r"(i32 %a116, i32 %s116)
  %a117 = add i32 %x, 117
  %s117 = sub i32 %y, 117
  call void asm sideeffect "", "r,r"(i32 %a117, i32 %s117)
  %a118 = add i32 %x, 118
  %s118 = sub i32 %y, 118
  call void asm sideeffect "", "r,r"(i32 %a118, i32 %s118)
  %a119 = add i32 %x, 119
  %s119 = sub i32 %y, 119
  call void asm sideeffect "", "r,r"(i32 %a119, i32 %s119)
  %a120 = add i32 %x, 120
  %s120 = sub i32 %y, 120
  call void asm sideeffect "", "r,r"(i32 %a120, i32 %s120)
  %a121 = add i32 %x, 121
  %s121 = sub i32 %y, 121
  call void asm sideeffect "", "r,r"(i32 %a121, i32 %s121)
  %a122 = add i32 %x, 122
  %s122 = sub i32 %y, 122
  call void asm sideeffect "", "r,r"(i32 %a122, i32 %s122)
  %a123 = add i32 %x, 123
  %s123 = sub i32 %y, 123
  call void asm sideeffect "", "r,r"(i32 %a123, i32 %s123)
  %a124 = add i32 %x, 124
  %s124 = sub i32 %y, 124
  call void asm sideeffect "", "r,r"(i32 %a124, i32 %s124)
  %a125 = add i32 %x, 125
  %s125 = sub i32 %y, 125
  call void asm sideeffect "", "r,r"(i32 %a125, i32 %s125)
  %a126 = add i32 %x, 126
  %s126 = sub i32 %y, 126
  call void asm sideeffect "", "r,r"(i32 %a126, i32 %s126)
  %a127 = add i32 %x, 127
  %s127 = sub i32 %y, 127
  call void asm sideeffect "", "r,r"(i32 %a127, i32 %s127)
  %a128 = add i32 %x, 128
  %s128 = sub i32 %y, 128
  call void asm sideeffect "", "r,r"(i32 %a128, i32 %s128)
  %a129 = add i32 %x, 129
  %s129 = sub i32 %y, 129
  call void asm sideeffect "", "r,r"(i32 %a129, i32 %s129)
  %a130 = add i32 %x, 130
  %s130 = sub i32 %y, 130
  call void asm sideeffect "", "r,r"(i32 %a130, i32 %s130)
  %a131 = add i32 %x, 131
  %s131 = sub i32 %y, 131
  call void asm sideeffect "", "r,r"(i32 %a131, i32 %s131)
  %a132 = add i32 %x, 132
  %s132 = sub i32 %y, 132
  call void asm sideeffect "", "r,r"(i32 %a132, i32 %s132)
  %a133 = add i32 %x, 133
  %s133 = sub i32 %y, 133
  call void asm sideeffect "", "r,r"(i32 %a133, i32 %s133)
  %a134 = add i32 %x, 134
  %s134 = sub i32 %y, 134
  call void asm sideeffect "", "r,r"(i32 %a134, i32 %s134)
  %a135 = add i32 %x, 135
  %s135 = sub i32 %y, 135
  call void asm sideeffect "", "r,r"(i32 %a135, i32 %s135)
  %a136 = add i32 %x, 136
  %s136 = sub i32 %y, 136
  call void asm sideeffect "", "r,r"(i32 %a136, i32 %s136)
  %a137 = add i32 %x, 137
  %s137 = sub i32 %y, 137
  call void asm sideeffect "", "r,r"(i32 %a137, i32 %s137)
  %a138 = add i32 %x, 138
  %s138 = sub i32 %y, 138
  call void asm sideeffect "", "r,r"(i32 %a138, i32 %s138)
  %a139 = add i32 %x, 139
  %s139 = sub i32 %y, 139
  call void asm sideeffect "", "r,r"(i32 %a139, i32 %s139)
  %a140 = add i32 %x, 140
  %s140 = sub i32 %y, 140
  call void asm sideeffect "", "r,r"(i32 %a140, i32 %s140)
  %a141 = add i32 %x, 141
  %s141 = sub i32 %y, 141
  call void asm sideeffect "", "r,r"(i32 %a141, i32 %s141)
  %a142 = add i32 %x, 142
  %s142 = sub i32 %y, 142
  call void asm sideeffect "", "r,r"(i32 %a142, i32 %s142)
  %a143 = add i32 %x, 143
  %s143 = sub i32 %y, 143
  call void asm sideeffect "", "r,r"(i32 %a143, i32 %s143)
  %a144 = add i32 %x, 144
  %s144 = sub i32 %y, 144
  call void asm sideeffect "", "r,r"(i32 %a144, i32 %s144)
  ret void
}
