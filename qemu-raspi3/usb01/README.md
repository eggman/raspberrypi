Simple example of USB host interrupt on QEMU raspi3 model. 

This program only supports QEMU raspi3 model, and does not work on real hardware.

```
$ make run
qemu-system-aarch64 -M raspi3 -m 1024 -serial null -serial mon:stdio -nographic -device usb-kbd -kernel kernel.elf
usb01
USB_CORE_GUID    00000000
USB_CORE_GSNPSID 4F54294A
HPRT 00021003
15000029 usb interrupt
Connector ID Status Change
```

If you use QEMU built with trace, you can output traces.

```
$ make trun
qemu-system-aarch64 -M raspi3 -m 1024 -serial null -serial mon:stdio -nographic -device usb-kbd -kernel kernel.elf -trace events=events
usb_port_claim bus 0, port 1
usb_hub_reset dev 0
usb_port_attach bus 0, port 1, devspeed full, portspeed full+high
usb_dwc2_attach port 0x7fef57f7e7d0
usb_dwc2_attach_speed full-speed device attached
usb_dwc2_bus_start start SOFs
usb_dwc2_raise_global_irq 0x01000000
usb_port_claim bus 0, port 1.1
usb_port_attach bus 0, port 1.1, devspeed full+high, portspeed full
usb_hub_attach dev 0, port 1
usb_dwc2_reset_enter === RESET enter ===
usb_dwc2_detach port 0x7fef57f7e7d0
usb_dwc2_bus_stop stop SOFs
usb_dwc2_bus_stop stop SOFs
usb_dwc2_reset_hold === RESET hold ===
usb_dwc2_reset_exit === RESET exit ===
usb_dwc2_attach port 0x7fef57f7e7d0
usb_dwc2_attach_speed full-speed device attached
usb_dwc2_bus_start start SOFs
usb_dwc2_raise_global_irq 0x01000000
usb_hub_reset dev 0
usb01
usb_dwc2_raise_global_irq 0x00000008
USB_CORE_GUID    usb_dwc2_glbreg_read  0x003c GUID      val 0x00000000
00000000
USB_CORE_GSNPSID usb_dwc2_glbreg_read  0x0040 GSNPSID   val 0x4f54294a
4F54294A
HPRT usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021003
00021003
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021003
usb_dwc2_hreg0_write  0x0440 HPRT0     val 0x00021003 old 0x00021003 result 0x00021001
usb_dwc2_hreg0_action disable PRTINT
usb_dwc2_lower_global_irq 0x01000000
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021001
usb_dwc2_hreg0_write  0x0440 HPRT0     val 0x00021001 old 0x00021001 result 0x00021001
usb_dwc2_hreg0_action disable PRTINT
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021001
usb_dwc2_hreg0_write  0x0440 HPRT0     val 0x00021101 old 0x00021001 result 0x00021101
usb_dwc2_hreg0_action disable PRTINT
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021101
usb_dwc2_hreg0_write  0x0440 HPRT0     val 0x00021101 old 0x00021101 result 0x00021101
usb_dwc2_hreg0_action disable PRTINT
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021101
usb_dwc2_hreg0_write  0x0440 HPRT0     val 0x00021105 old 0x00021101 result 0x00021101
usb_dwc2_hreg0_action disable PRTINT
usb_dwc2_glbreg_read  0x0008 GAHBCFG   val 0x00000000
usb_dwc2_glbreg_write 0x0008 GAHBCFG   val 0x00000001 old 0x00000000 result 0x00000001
usb_dwc2_glbreg_write 0x0018 GINTMSK   val 0x10000000 old 0x00000000 result 0x10000000
usb_dwc2_update_irq level=1
usb_dwc2_hreg0_write  0x0440 HPRT0     val 0x0000000a old 0x00021101 result 0x0002000d
usb_dwc2_hreg0_action call usb_port_reset
usb_dwc2_detach port 0x7fef57f7e7d0
usb_dwc2_bus_stop stop SOFs
usb_dwc2_raise_global_irq 0x01000000
usb_dwc2_attach port 0x7fef57f7e7d0
usb_dwc2_attach_speed full-speed device attached
usb_dwc2_bus_start start SOFs
usb_hub_reset dev 0
usb_dwc2_hreg0_action enable PRTINT
usb_dwc2_hreg1_read   0x0500 HCCHAR  40 val 0x00000000
usb_dwc2_hreg1_write  0x0500 HCCHAR  0 val 0x80000000 old 0x00000000 result 0x80000000
usb_dwc2_find_device 0
usb_dwc2_device_found device found on port 0
usb_dwc2_enable_chan ch 0 dev 0x55c602be63c0 pkt 0x7fef57f7e818 ep 0
usb_dwc2_handle_packet ch 0 dev 0x55c602be63c0 pkt 0x7fef57f7e818 ep 0 type Ctrl dir Out mps 0 len 0 pcnt 0
usb_dwc2_work_bh
usb_dwc2_glbreg_read  0x0014 GINTSTS   val 0x15000029
usb_dwc2_glbreg_write 0x0014 GINTSTS   val 0x15000029 old 0x15000029 result 0xffffffff05000021
usb_dwc2_update_irq level=0
15000029 usb interrupt
Connector ID Status Change
usb_dwc2_raise_global_irq 0x00000008
```
