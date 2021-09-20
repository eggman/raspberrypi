USB host send control packet tx / rx and display GET_DESCRIPTOR result.

This program only supports QEMU raspi3 model, and does not work on real hardware.

```
$ make run
qemu-system-aarch64 -M raspi3 -m 1024 -serial null -serial mon:stdio -nographic -device usb-kbd -kernel kernel.elf
qemu exit: Ctrl-A x / qemu monitor: Ctrl-A c
usb02
usb_buffer0:00083100
usb_buffer1:00083000
USB_CORE_GUID    00000000
USB_CORE_GSNPSID 4F54294A
14000029 usb interrupt
Connector ID Status Change
Periodic TxFIFO Empty
Non-periodic TxFIFO Empty
Host and Device Start of Frame
GET_DESCRIPTOR
0000000A
00000029
00000008
0000000A
00000000
00000001
00000000
00000000
00000000
000000FF
00000000
00000000
00000000
00000000
00000000
00000000
00000000
00000000
```

If you use QEMU built with trace, you can output traces.

```
$ make trun
qemu-system-aarch64 -M raspi3 -m 1024 -serial null -serial mon:stdio -nographic -device usb-kbd -kernel kernel.elf -trace events=events
usb_port_claim bus 0, port 1
usb_hub_reset dev 0
usb_port_attach bus 0, port 1, devspeed full, portspeed full+high
usb_dwc2_attach port 0x7f16f50c77d0
usb_dwc2_attach_speed full-speed device attached
usb_dwc2_bus_start start SOFs
usb_dwc2_raise_global_irq 0x01000000
usb_port_claim bus 0, port 1.1
usb_port_attach bus 0, port 1.1, devspeed full+high, portspeed full
usb_hub_attach dev 0, port 1
usb_dwc2_reset_enter === RESET enter ===
usb_dwc2_detach port 0x7f16f50c77d0
usb_dwc2_bus_stop stop SOFs
usb_dwc2_bus_stop stop SOFs
usb_dwc2_reset_hold === RESET hold ===
usb_dwc2_reset_exit === RESET exit ===
usb_dwc2_attach port 0x7f16f50c77d0
usb_dwc2_attach_speed full-speed device attached
usb_dwc2_bus_start start SOFs
usb_dwc2_raise_global_irq 0x01000000
usb_hub_reset dev 0
qemu exit: Ctrl-A x / qemu monitor: Ctrl-A c
usb02
usb_buffer0:usb_dwc2_raise_global_irq 0x00000008
00083100
usb_buffer1:00083000
USB_CORE_GUID    usb_dwc2_glbreg_read  0x003c GUID      val 0x00000000
00000000
USB_CORE_GSNPSID usb_dwc2_glbreg_read  0x0040 GSNPSID   val 0x4f54294a
4F54294A
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021003
usb_dwc2_hreg0_write  0x0440 HPRT0     val 0x00021003 old 0x00021003 result 0x00021001
usb_dwc2_hreg0_action disable PRTINT
usb_dwc2_lower_global_irq 0x01000000
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021001
usb_dwc2_hreg0_write  0x0440 HPRT0     val 0x00021101 old 0x00021001 result 0x00021101
usb_dwc2_hreg0_action disable PRTINT
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021101
usb_dwc2_hreg0_write  0x0440 HPRT0     val 0x00021001 old 0x00021101 result 0x0002100d
usb_dwc2_hreg0_action call usb_port_reset
usb_dwc2_detach port 0x7f16f50c77d0
usb_dwc2_bus_stop stop SOFs
usb_dwc2_raise_global_irq 0x01000000
usb_dwc2_attach port 0x7f16f50c77d0
usb_dwc2_attach_speed full-speed device attached
usb_dwc2_bus_start start SOFs
usb_hub_reset dev 0
usb_dwc2_hreg0_action enable PRTINT
usb_dwc2_glbreg_read  0x0008 GAHBCFG   val 0x00000000
usb_dwc2_glbreg_write 0x0008 GAHBCFG   val 0x00000001 old 0x00000000 result 0x00000001
usb_dwc2_glbreg_write 0x0018 GINTMSK   val 0x11000000 old 0x00000000 result 0x11000000
usb_dwc2_update_irq level=1
usb_dwc2_hreg0_write  0x0440 HPRT0     val 0x0000100a old 0x0002100d result 0x00021005
usb_dwc2_hreg0_action disable PRTINT
usb_dwc2_lower_global_irq 0x01000000
usb_dwc2_hreg0_read   0x0418 HAINTMSK  val 0x00000000
usb_dwc2_hreg0_write  0x0418 HAINTMSK  val 0x00000003 old 0x00000000 result 0x00000003
usb_dwc2_hreg1_read   0x050c HCINTMSK40 val 0x00000000
usb_dwc2_hreg1_write  0x050c HCINTMSK0 val 0x00000001 old 0x00000000 result 0x00000001
usb_dwc2_hreg1_read   0x052c HCINTMSK41 val 0x00000000
usb_dwc2_hreg1_write  0x052c HCINTMSK1 val 0x00000001 old 0x00000000 result 0x00000001
usb_dwc2_hreg1_read   0x0500 HCCHAR  40 val 0x00000000
usb_dwc2_hreg1_write  0x0500 HCCHAR  0 val 0x00000040 old 0x00000000 result 0x00000040
usb_dwc2_hreg1_read   0x0520 HCCHAR  41 val 0x00000000
usb_dwc2_hreg1_write  0x0520 HCCHAR  1 val 0x00008040 old 0x00000000 result 0x00008040
usb_dwc2_glbreg_read  0x0014 GINTSTS   val 0x14000029
usb_dwc2_glbreg_write 0x0014 GINTSTS   val 0x14000029 old 0x14000029 result 0xffffffff04000021
usb_dwc2_update_irq level=0
14000029usb_dwc2_raise_global_irq 0x00000008
 usb interrupt
Connector ID Status Change
Periodic TxFIFO Empty
Non-periodic TxFIFO Empty
Host and Device Start of Frame
usb_dwc2_hreg1_read   0x0500 HCCHAR  40 val 0x00000040
usb_dwc2_hreg1_write  0x0500 HCCHAR  0 val 0x00000040 old 0x00000040 result 0x00000040
usb_dwc2_hreg1_read   0x0520 HCCHAR  41 val 0x00008040
usb_dwc2_hreg1_write  0x0520 HCCHAR  1 val 0x00008040 old 0x00008040 result 0x00008040
usb_dwc2_hreg1_write  0x0514 HCDMA   0 val 0x00083100 old 0x00000000 result 0x00083100
usb_dwc2_hreg1_read   0x0514 HCDMA   40 val 0x00083100
usb_dwc2_hreg1_write  0x0514 HCDMA   0 val 0xc0083100 old 0x00083100 result 0xc0083100
usb_dwc2_hreg1_write  0x0510 HCTSIZ  0 val 0x60080008 old 0x00000000 result 0x60080008
usb_dwc2_hreg1_read   0x0500 HCCHAR  40 val 0x00000040
usb_dwc2_hreg1_write  0x0500 HCCHAR  0 val 0x80000040 old 0x00000040 result 0x80000040
usb_dwc2_find_device 0
usb_dwc2_device_found device found on port 0
usb_dwc2_enable_chan ch 0 dev 0x55df43b7cdb0 pkt 0x7f16f50c7818 ep 0
usb_dwc2_handle_packet ch 0 dev 0x55df43b7cdb0 pkt 0x7f16f50c7818 ep 0 type Ctrl dir Out mps 64 len 8 pcnt 1
usb_dwc2_memory_read addr -1073204992 len 8
usb_packet_state_change bus 0, port 1, ep 0, packet 0x7f16f50c7818, state undef -> setup
usb_hub_control dev 0, req 0xa006, value 256, index 0, langth 64
usb_packet_state_change bus 0, port 1, ep 0, packet 0x7f16f50c7818, state setup -> complete
usb_dwc2_packet_status status USB_RET_SUCCESS len 8
usb_dwc2_packet_done status USB_RET_SUCCESS actual 8 len 0 pcnt 0
usb_dwc2_raise_host_irq 0x0001
usb_dwc2_raise_global_irq 0x02000000
usb_dwc2_hreg1_write  0x0534 HCDMA   1 val 0x00083000 old 0x00000000 result 0x00083000
usb_dwc2_hreg1_read   0x0534 HCDMA   41 val 0x00083000
usb_dwc2_hreg1_write  0x0534 HCDMA   1 val 0xc0083000 old 0x00083000 result 0xc0083000
usb_dwc2_hreg1_write  0x0530 HCTSIZ  1 val 0x40080040 old 0x00000000 result 0x40080040
usb_dwc2_hreg1_read   0x0520 HCCHAR  41 val 0x00008040
usb_dwc2_hreg1_write  0x0520 HCCHAR  1 val 0x80008040 old 0x00008040 result 0x80008040
usb_dwc2_find_device 0
usb_dwc2_device_found device found on port 0
usb_dwc2_enable_chan ch 1 dev 0x55df43b7cdb0 pkt 0x7f16f50c78c8 ep 0
usb_dwc2_handle_packet ch 1 dev 0x55df43b7cdb0 pkt 0x7f16f50c78c8 ep 0 type Ctrl dir In mps 64 len 64 pcnt 1
usb_packet_state_change bus 0, port 1, ep 0, packet 0x7f16f50c78c8, state undef -> setup
usb_packet_state_change bus 0, port 1, ep 0, packet 0x7f16f50c78c8, state setup -> complete
usb_dwc2_packet_status status USB_RET_SUCCESS len 10
usb_dwc2_memory_write addr -1073205248 len 10
usb_dwc2_packet_done status USB_RET_SUCCESS actual 10 len 54 pcnt 0
usb_dwc2_raise_host_irq 0x0002
GET_DESCRIPTOR
0000000A
00000029
00000008
0000000A
00000000
00000001
00000000
00000000
00000000
000000FF
00000000
00000000
00000000
00000000
00000000
00000000
00000000
00000000
usb_dwc2_work_bh
```
