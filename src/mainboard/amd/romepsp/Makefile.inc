subdirs-y += ../../../cpu/amd/mtrr/
subdirs-y += ../../../cpu/x86/tsc
subdirs-y += ../../../cpu/x86/lapic
subdirs-y += ../../../cpu/x86/cache
subdirs-y += ../../../cpu/x86/mtrr
subdirs-y += ../../../cpu/x86/pae
# subdirs-y += ../../../cpu/x86/smm
subdirs-y += ../../../cpu/x86/name
#subdirs-y += ../../../cpu/x86/


# ewwwww ...
subdirs-y += ../../../drivers/generic/ioapic

bootblock-y += ../../../soc/amd/common/block/spi/fch_spi.c ../../..//soc/amd/picasso/config.c ../../..//soc/amd/picasso/gpio.c ../../..//soc/amd/romepsp/i2c.c ../../..//soc/amd/picasso/smi_util.c ../../..//soc/amd/picasso/southbridge.c ../../..//soc/amd/romepsp/uart.c ../../..//soc/amd/picasso/bootblock/bootblock.c
#bootblock-y += bootblock.c
#bootblock-y += uart.c

romstage-y += fw_cfg.c
romstage-y += memmap.c
romstage-y += ../../../soc/amd/common/block/spi/fch_spi.c ../../..//soc/amd/picasso/config.c ../../..//soc/amd/picasso/gpio.c ../../..//soc/amd/romepsp/i2c.c ../../..//soc/amd/picasso/smi_util.c ../../..//soc/amd/picasso/southbridge.c ../../..//soc/amd/romepsp/uart.c

postcar-y += fw_cfg.c
postcar-y += memmap.c
postcar-y += exit_car.S
postcar-y += ../../../soc/amd/common/block/spi/fch_spi.c ../../..//soc/amd/picasso/config.c ../../..//soc/amd/picasso/gpio.c ../../..//soc/amd/romepsp/i2c.c ../../..//soc/amd/picasso/smi_util.c ../../..//soc/amd/picasso/southbridge.c ../../..//soc/amd/romepsp/uart.c

ramstage-y += fw_cfg.c
ramstage-y += memmap.c
ramstage-y += ../../../soc/amd/common/block/spi/fch_spi.c ../../..//soc/amd/picasso/config.c ../../..//soc/amd/picasso/gpio.c ../../..//soc/amd/romepsp/i2c.c ../../..//soc/amd/picasso/smi_util.c ../../..//soc/amd/picasso/southbridge.c ../../..//soc/amd/romepsp/uart.c
ramstage-y += ../../..//soc/amd/romepsp/acpi.c
ramstage-y += ../../..//soc/amd/picasso/psp.c
ramstage-y += ../../..//soc/amd/romepsp/cpu.c
ramstage-y += ../../..//soc/amd/romepsp/chip.c
ramstage-y += ../../..//soc/amd/romepsp/root_complex.c
ramstage-y += ../../..//soc/amd/romepsp/data_fabric_util.c
#ramstage-y += ../../..//cpu/x86/mp_init.c

CPPFLAGS_common += -I$(src)/soc/amd/common/block/include
CPPFLAGS_common += -I$(src)/soc/amd/picasso/include
