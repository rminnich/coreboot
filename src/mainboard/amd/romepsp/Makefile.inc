bootblock-y += ../../../soc/amd/common/block/spi/fch_spi.c ../../..//soc/amd/picasso/config.c ../../..//soc/amd/picasso/gpio.c ../../..//soc/amd/picasso/i2c.c ../../..//soc/amd/picasso/smi_util.c ../../..//soc/amd/picasso/southbridge.c ../../..//soc/amd/picasso/uart.c
bootblock-y += bootblock.c
#bootblock-y += uart.c

romstage-y += fw_cfg.c
romstage-y += memmap.c
romstage-y += ../../../soc/amd/common/block/spi/fch_spi.c ../../..//soc/amd/picasso/config.c ../../..//soc/amd/picasso/gpio.c ../../..//soc/amd/picasso/i2c.c ../../..//soc/amd/picasso/smi_util.c ../../..//soc/amd/picasso/southbridge.c ../../..//soc/amd/picasso/uart.c

postcar-y += fw_cfg.c
postcar-y += memmap.c
postcar-y += exit_car.S

ramstage-y += fw_cfg.c
ramstage-y += memmap.c
ramstage-y += ../../../soc/amd/common/block/spi/fch_spi.c ../../..//soc/amd/picasso/config.c ../../..//soc/amd/picasso/gpio.c ../../..//soc/amd/picasso/i2c.c ../../..//soc/amd/picasso/smi_util.c ../../..//soc/amd/picasso/southbridge.c ../../..//soc/amd/picasso/uart.c
ramstage-y += ../../..//soc/amd/picasso/acpi.c

CPPFLAGS_common += -I$(src)/soc/amd/common/block/include
CPPFLAGS_common += -I$(src)/soc/amd/picasso/include
