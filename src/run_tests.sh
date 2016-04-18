#!/bin/bash

PASS=0
FAIL=0
EXECUTED=0

function test {
    "$@"
    local status=$?
    
    if [ $status -ne 0 ]; then
        echo "ERROR WITH $2" >&2
        let FAIL=FAIL+1
    fi
    if [ $status -eq 0 ]; then
        let PASS=PASS+1
    fi
    let EXECUTED=EXECUTED+1
    return $status
}

function all {
    test ./gb_emu ../../tests/mooneye/emulator-only/mbc1_rom_4banks.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/call_cc_timing2.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/div_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/pop_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/intr_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/call_cc_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/rapid_di_ei.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/call_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/add_sp_e_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/halt_ime1_timing2-GS.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/rst_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/halt_ime0_nointr_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/ret_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/div_write.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/rapid_toggle.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim00.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim00_div_trigger.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim01.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim01_div_trigger.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim10.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim10_div_trigger.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim11.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim11_div_trigger.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tima_reload.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tima_write_reloading.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tma_write_reloading.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/reti_intr_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/oam_dma_restart.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/jp_cc_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/ret_cc_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_0_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_oam_ok_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_mode3_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_1_2_timing-GS.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_mode0_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/vblank_stat_intr-GS.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_mode0_timing_sprites.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/hblank_ly_scx_timing-GS.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/jp_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/bits/mem_oam.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/bits/reg_f.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/bits/unused_hwio-GS.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/ei_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/oam_dma_start.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/ld_hl_sp_e_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/call_timing2.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/oam_dma_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/halt_ime0_ei.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/halt_ime1_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/boot_regs-dmg.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/reti_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/if_ie_registers.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/di_timing-GS.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/push_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/boot_hwio-G.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/manual-only/sprite_priority.gb > /dev/null
}

function gpu {
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_0_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_oam_ok_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_mode3_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_1_2_timing-GS.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_mode0_timing.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/vblank_stat_intr-GS.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/intr_2_mode0_timing_sprites.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/gpu/hblank_ly_scx_timing-GS.gb > /dev/null
}

function timer {
    test ./gb_emu ../../tests/mooneye/acceptance/timer/div_write.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/rapid_toggle.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim00.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim00_div_trigger.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim01.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim01_div_trigger.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim10.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim10_div_trigger.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim11.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tim11_div_trigger.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tima_reload.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tima_write_reloading.gb > /dev/null
    test ./gb_emu ../../tests/mooneye/acceptance/timer/timer_tma_write_reloading.gb > /dev/null
}

function blargg {
    test ./gb_emu ../../tests/blargg/cpu_instrs/cpu_instrs.gb > /dev/null 
    test ./gb_emu ../../tests/blargg/instr_timing/instr_timing.gb > /dev/null
    test ./gb_emu ../../tests/blargg/mem_timing/mem_timing.gb > /dev/null
    test ./gb_emu ../../tests/blargg/mem_timing-2/mem_timing.gb > /dev/null
    test ./gb_emu ../../tests/blargg/oam_bug/oam_bug.gb > /dev/null
    test ./gb_emu ../../tests/blargg/oam_bug-2/oam_bug.gb > /dev/null
}

if [ $# -eq 0 ]
  then
    echo "USAGE"
    echo "all      run all mooneye tests"
    echo "timer    run all mooneye timer tests"
    echo "gpu      run all mooneye gpu tests"
    echo "blargg   run all blargg tests"
    echo "         use [esc] if successfull"
    echo "         use [F1]  if not successfull"
    exit
fi

case $1 in
    'all') 
        all;;
    'timer') 
        timer;;
    'gpu') 
        gpu;;
    'blargg') 
        blargg;;
esac 

echo "EXECUTED $EXECUTED TEST CASES"
echo "PASSed = $PASS, FAILED = $FAIL"
exit