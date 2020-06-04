bootblock-y += bootblock.c
bootblock-y += uart.c

romstage-y += fw_cfg.c
romstage-y += memmap.c

postcar-y += fw_cfg.c
postcar-y += memmap.c
postcar-y += exit_car.S

ramstage-y += fw_cfg.c
ramstage-y += memmap.c

CPPFLAGS_common += -I$(src)/soc/amd/common/block/include
CPPFLAGS_common += -I$(src)/soc/amd/picasso/include
