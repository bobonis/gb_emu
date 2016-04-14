#!/bin/bash

function test {
    "$@"
    local status=$?
    
    if [ $status -ne 0 ]; then
        echo "ERROR WITH $2" >&2
    fi
    return $status
}


#test ./gb_emu ../../tests/blargg/cpu_instrs/cpu_instrs.gb > /dev/null 
#test ./gb_emu ../../tests/blargg/instr_timing/instr_timing.gb > /dev/null
#test ./gb_emu ../../tests/blargg/mem_timing/mem_timing.gb > /dev/null
#test ./gb_emu ../../tests/blargg/mem_timing-2/mem_timing.gb > /dev/null
#test ./gb_emu ../../tests/blargg/oam_bug/oam_bug.gb > /dev/null
#test ./gb_emu ../../tests/blargg/oam_bug-2/oam_bug.gb > /dev/null

#test ./gb_emu ../../tests/mooneye/emulator-only/mbc1_rom_4banks.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/call_cc_timing2.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/div_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/pop_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/intr_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/call_cc_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/rapid_di_ei.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/call_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/add_sp_e_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/halt_ime1_timing2-GS.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/rst_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/halt_ime0_nointr_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/ret_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/timer/div_write.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/timer/rapid_toggle.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/reti_intr_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/oam_dma_restart.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/jp_cc_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/ret_cc_timing.gb > /dev/null
test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_0_timing.gb > /dev/null
test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_oam_ok_timing.gb > /dev/null
test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_mode3_timing.gb > /dev/null
test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_1_2_timing-GS.gb > /dev/null
test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_mode0_timing.gb > /dev/null
test ./gb_emu ../../tests/mooneye/acceptance/gpu/vblank_stat_intr-GS.gb > /dev/null
test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_mode0_timing_sprites.gb > /dev/null
test ./gb_emu ../../tests/mooneye/acceptance/gpu/hblank_ly_scx_timing-GS.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/jp_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/bits/mem_oam.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/bits/reg_f.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/bits/unused_hwio-GS.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/ei_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/oam_dma_start.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/ld_hl_sp_e_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/call_timing2.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/oam_dma_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/halt_ime0_ei.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/halt_ime1_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/boot_regs-dmg.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/reti_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/if_ie_registers.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/di_timing-GS.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/push_timing.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/acceptance/boot_hwio-G.gb > /dev/null
#test ./gb_emu ../../tests/mooneye/manual-only/sprite_priority.gb > /dev/null




