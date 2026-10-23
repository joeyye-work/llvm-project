; RUN: llc -mtriple=groq-unknown-unknown -mcpu=tsp-v1 -verify-machineinstrs < %s | FileCheck %s

define void @queue_pool_144_add(i32 %x) {
; CHECK-LABEL: queue_pool_144_add:
; CHECK-COUNT-144: add
  %v1 = add i32 %x, 1
  call void asm sideeffect "", "r"(i32 %v1)
  %v2 = add i32 %x, 2
  call void asm sideeffect "", "r"(i32 %v2)
  %v3 = add i32 %x, 3
  call void asm sideeffect "", "r"(i32 %v3)
  %v4 = add i32 %x, 4
  call void asm sideeffect "", "r"(i32 %v4)
  %v5 = add i32 %x, 5
  call void asm sideeffect "", "r"(i32 %v5)
  %v6 = add i32 %x, 6
  call void asm sideeffect "", "r"(i32 %v6)
  %v7 = add i32 %x, 7
  call void asm sideeffect "", "r"(i32 %v7)
  %v8 = add i32 %x, 8
  call void asm sideeffect "", "r"(i32 %v8)
  %v9 = add i32 %x, 9
  call void asm sideeffect "", "r"(i32 %v9)
  %v10 = add i32 %x, 10
  call void asm sideeffect "", "r"(i32 %v10)
  %v11 = add i32 %x, 11
  call void asm sideeffect "", "r"(i32 %v11)
  %v12 = add i32 %x, 12
  call void asm sideeffect "", "r"(i32 %v12)
  %v13 = add i32 %x, 13
  call void asm sideeffect "", "r"(i32 %v13)
  %v14 = add i32 %x, 14
  call void asm sideeffect "", "r"(i32 %v14)
  %v15 = add i32 %x, 15
  call void asm sideeffect "", "r"(i32 %v15)
  %v16 = add i32 %x, 16
  call void asm sideeffect "", "r"(i32 %v16)
  %v17 = add i32 %x, 17
  call void asm sideeffect "", "r"(i32 %v17)
  %v18 = add i32 %x, 18
  call void asm sideeffect "", "r"(i32 %v18)
  %v19 = add i32 %x, 19
  call void asm sideeffect "", "r"(i32 %v19)
  %v20 = add i32 %x, 20
  call void asm sideeffect "", "r"(i32 %v20)
  %v21 = add i32 %x, 21
  call void asm sideeffect "", "r"(i32 %v21)
  %v22 = add i32 %x, 22
  call void asm sideeffect "", "r"(i32 %v22)
  %v23 = add i32 %x, 23
  call void asm sideeffect "", "r"(i32 %v23)
  %v24 = add i32 %x, 24
  call void asm sideeffect "", "r"(i32 %v24)
  %v25 = add i32 %x, 25
  call void asm sideeffect "", "r"(i32 %v25)
  %v26 = add i32 %x, 26
  call void asm sideeffect "", "r"(i32 %v26)
  %v27 = add i32 %x, 27
  call void asm sideeffect "", "r"(i32 %v27)
  %v28 = add i32 %x, 28
  call void asm sideeffect "", "r"(i32 %v28)
  %v29 = add i32 %x, 29
  call void asm sideeffect "", "r"(i32 %v29)
  %v30 = add i32 %x, 30
  call void asm sideeffect "", "r"(i32 %v30)
  %v31 = add i32 %x, 31
  call void asm sideeffect "", "r"(i32 %v31)
  %v32 = add i32 %x, 32
  call void asm sideeffect "", "r"(i32 %v32)
  %v33 = add i32 %x, 33
  call void asm sideeffect "", "r"(i32 %v33)
  %v34 = add i32 %x, 34
  call void asm sideeffect "", "r"(i32 %v34)
  %v35 = add i32 %x, 35
  call void asm sideeffect "", "r"(i32 %v35)
  %v36 = add i32 %x, 36
  call void asm sideeffect "", "r"(i32 %v36)
  %v37 = add i32 %x, 37
  call void asm sideeffect "", "r"(i32 %v37)
  %v38 = add i32 %x, 38
  call void asm sideeffect "", "r"(i32 %v38)
  %v39 = add i32 %x, 39
  call void asm sideeffect "", "r"(i32 %v39)
  %v40 = add i32 %x, 40
  call void asm sideeffect "", "r"(i32 %v40)
  %v41 = add i32 %x, 41
  call void asm sideeffect "", "r"(i32 %v41)
  %v42 = add i32 %x, 42
  call void asm sideeffect "", "r"(i32 %v42)
  %v43 = add i32 %x, 43
  call void asm sideeffect "", "r"(i32 %v43)
  %v44 = add i32 %x, 44
  call void asm sideeffect "", "r"(i32 %v44)
  %v45 = add i32 %x, 45
  call void asm sideeffect "", "r"(i32 %v45)
  %v46 = add i32 %x, 46
  call void asm sideeffect "", "r"(i32 %v46)
  %v47 = add i32 %x, 47
  call void asm sideeffect "", "r"(i32 %v47)
  %v48 = add i32 %x, 48
  call void asm sideeffect "", "r"(i32 %v48)
  %v49 = add i32 %x, 49
  call void asm sideeffect "", "r"(i32 %v49)
  %v50 = add i32 %x, 50
  call void asm sideeffect "", "r"(i32 %v50)
  %v51 = add i32 %x, 51
  call void asm sideeffect "", "r"(i32 %v51)
  %v52 = add i32 %x, 52
  call void asm sideeffect "", "r"(i32 %v52)
  %v53 = add i32 %x, 53
  call void asm sideeffect "", "r"(i32 %v53)
  %v54 = add i32 %x, 54
  call void asm sideeffect "", "r"(i32 %v54)
  %v55 = add i32 %x, 55
  call void asm sideeffect "", "r"(i32 %v55)
  %v56 = add i32 %x, 56
  call void asm sideeffect "", "r"(i32 %v56)
  %v57 = add i32 %x, 57
  call void asm sideeffect "", "r"(i32 %v57)
  %v58 = add i32 %x, 58
  call void asm sideeffect "", "r"(i32 %v58)
  %v59 = add i32 %x, 59
  call void asm sideeffect "", "r"(i32 %v59)
  %v60 = add i32 %x, 60
  call void asm sideeffect "", "r"(i32 %v60)
  %v61 = add i32 %x, 61
  call void asm sideeffect "", "r"(i32 %v61)
  %v62 = add i32 %x, 62
  call void asm sideeffect "", "r"(i32 %v62)
  %v63 = add i32 %x, 63
  call void asm sideeffect "", "r"(i32 %v63)
  %v64 = add i32 %x, 64
  call void asm sideeffect "", "r"(i32 %v64)
  %v65 = add i32 %x, 65
  call void asm sideeffect "", "r"(i32 %v65)
  %v66 = add i32 %x, 66
  call void asm sideeffect "", "r"(i32 %v66)
  %v67 = add i32 %x, 67
  call void asm sideeffect "", "r"(i32 %v67)
  %v68 = add i32 %x, 68
  call void asm sideeffect "", "r"(i32 %v68)
  %v69 = add i32 %x, 69
  call void asm sideeffect "", "r"(i32 %v69)
  %v70 = add i32 %x, 70
  call void asm sideeffect "", "r"(i32 %v70)
  %v71 = add i32 %x, 71
  call void asm sideeffect "", "r"(i32 %v71)
  %v72 = add i32 %x, 72
  call void asm sideeffect "", "r"(i32 %v72)
  %v73 = add i32 %x, 73
  call void asm sideeffect "", "r"(i32 %v73)
  %v74 = add i32 %x, 74
  call void asm sideeffect "", "r"(i32 %v74)
  %v75 = add i32 %x, 75
  call void asm sideeffect "", "r"(i32 %v75)
  %v76 = add i32 %x, 76
  call void asm sideeffect "", "r"(i32 %v76)
  %v77 = add i32 %x, 77
  call void asm sideeffect "", "r"(i32 %v77)
  %v78 = add i32 %x, 78
  call void asm sideeffect "", "r"(i32 %v78)
  %v79 = add i32 %x, 79
  call void asm sideeffect "", "r"(i32 %v79)
  %v80 = add i32 %x, 80
  call void asm sideeffect "", "r"(i32 %v80)
  %v81 = add i32 %x, 81
  call void asm sideeffect "", "r"(i32 %v81)
  %v82 = add i32 %x, 82
  call void asm sideeffect "", "r"(i32 %v82)
  %v83 = add i32 %x, 83
  call void asm sideeffect "", "r"(i32 %v83)
  %v84 = add i32 %x, 84
  call void asm sideeffect "", "r"(i32 %v84)
  %v85 = add i32 %x, 85
  call void asm sideeffect "", "r"(i32 %v85)
  %v86 = add i32 %x, 86
  call void asm sideeffect "", "r"(i32 %v86)
  %v87 = add i32 %x, 87
  call void asm sideeffect "", "r"(i32 %v87)
  %v88 = add i32 %x, 88
  call void asm sideeffect "", "r"(i32 %v88)
  %v89 = add i32 %x, 89
  call void asm sideeffect "", "r"(i32 %v89)
  %v90 = add i32 %x, 90
  call void asm sideeffect "", "r"(i32 %v90)
  %v91 = add i32 %x, 91
  call void asm sideeffect "", "r"(i32 %v91)
  %v92 = add i32 %x, 92
  call void asm sideeffect "", "r"(i32 %v92)
  %v93 = add i32 %x, 93
  call void asm sideeffect "", "r"(i32 %v93)
  %v94 = add i32 %x, 94
  call void asm sideeffect "", "r"(i32 %v94)
  %v95 = add i32 %x, 95
  call void asm sideeffect "", "r"(i32 %v95)
  %v96 = add i32 %x, 96
  call void asm sideeffect "", "r"(i32 %v96)
  %v97 = add i32 %x, 97
  call void asm sideeffect "", "r"(i32 %v97)
  %v98 = add i32 %x, 98
  call void asm sideeffect "", "r"(i32 %v98)
  %v99 = add i32 %x, 99
  call void asm sideeffect "", "r"(i32 %v99)
  %v100 = add i32 %x, 100
  call void asm sideeffect "", "r"(i32 %v100)
  %v101 = add i32 %x, 101
  call void asm sideeffect "", "r"(i32 %v101)
  %v102 = add i32 %x, 102
  call void asm sideeffect "", "r"(i32 %v102)
  %v103 = add i32 %x, 103
  call void asm sideeffect "", "r"(i32 %v103)
  %v104 = add i32 %x, 104
  call void asm sideeffect "", "r"(i32 %v104)
  %v105 = add i32 %x, 105
  call void asm sideeffect "", "r"(i32 %v105)
  %v106 = add i32 %x, 106
  call void asm sideeffect "", "r"(i32 %v106)
  %v107 = add i32 %x, 107
  call void asm sideeffect "", "r"(i32 %v107)
  %v108 = add i32 %x, 108
  call void asm sideeffect "", "r"(i32 %v108)
  %v109 = add i32 %x, 109
  call void asm sideeffect "", "r"(i32 %v109)
  %v110 = add i32 %x, 110
  call void asm sideeffect "", "r"(i32 %v110)
  %v111 = add i32 %x, 111
  call void asm sideeffect "", "r"(i32 %v111)
  %v112 = add i32 %x, 112
  call void asm sideeffect "", "r"(i32 %v112)
  %v113 = add i32 %x, 113
  call void asm sideeffect "", "r"(i32 %v113)
  %v114 = add i32 %x, 114
  call void asm sideeffect "", "r"(i32 %v114)
  %v115 = add i32 %x, 115
  call void asm sideeffect "", "r"(i32 %v115)
  %v116 = add i32 %x, 116
  call void asm sideeffect "", "r"(i32 %v116)
  %v117 = add i32 %x, 117
  call void asm sideeffect "", "r"(i32 %v117)
  %v118 = add i32 %x, 118
  call void asm sideeffect "", "r"(i32 %v118)
  %v119 = add i32 %x, 119
  call void asm sideeffect "", "r"(i32 %v119)
  %v120 = add i32 %x, 120
  call void asm sideeffect "", "r"(i32 %v120)
  %v121 = add i32 %x, 121
  call void asm sideeffect "", "r"(i32 %v121)
  %v122 = add i32 %x, 122
  call void asm sideeffect "", "r"(i32 %v122)
  %v123 = add i32 %x, 123
  call void asm sideeffect "", "r"(i32 %v123)
  %v124 = add i32 %x, 124
  call void asm sideeffect "", "r"(i32 %v124)
  %v125 = add i32 %x, 125
  call void asm sideeffect "", "r"(i32 %v125)
  %v126 = add i32 %x, 126
  call void asm sideeffect "", "r"(i32 %v126)
  %v127 = add i32 %x, 127
  call void asm sideeffect "", "r"(i32 %v127)
  %v128 = add i32 %x, 128
  call void asm sideeffect "", "r"(i32 %v128)
  %v129 = add i32 %x, 129
  call void asm sideeffect "", "r"(i32 %v129)
  %v130 = add i32 %x, 130
  call void asm sideeffect "", "r"(i32 %v130)
  %v131 = add i32 %x, 131
  call void asm sideeffect "", "r"(i32 %v131)
  %v132 = add i32 %x, 132
  call void asm sideeffect "", "r"(i32 %v132)
  %v133 = add i32 %x, 133
  call void asm sideeffect "", "r"(i32 %v133)
  %v134 = add i32 %x, 134
  call void asm sideeffect "", "r"(i32 %v134)
  %v135 = add i32 %x, 135
  call void asm sideeffect "", "r"(i32 %v135)
  %v136 = add i32 %x, 136
  call void asm sideeffect "", "r"(i32 %v136)
  %v137 = add i32 %x, 137
  call void asm sideeffect "", "r"(i32 %v137)
  %v138 = add i32 %x, 138
  call void asm sideeffect "", "r"(i32 %v138)
  %v139 = add i32 %x, 139
  call void asm sideeffect "", "r"(i32 %v139)
  %v140 = add i32 %x, 140
  call void asm sideeffect "", "r"(i32 %v140)
  %v141 = add i32 %x, 141
  call void asm sideeffect "", "r"(i32 %v141)
  %v142 = add i32 %x, 142
  call void asm sideeffect "", "r"(i32 %v142)
  %v143 = add i32 %x, 143
  call void asm sideeffect "", "r"(i32 %v143)
  %v144 = add i32 %x, 144
  call void asm sideeffect "", "r"(i32 %v144)
  ret void
}
