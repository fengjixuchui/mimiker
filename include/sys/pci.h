#ifndef _SYS_PCI_H_
#define _SYS_PCI_H_

#include <sys/cdefs.h>
#include <sys/device.h>
#include <sys/bus.h>

typedef struct {
  uint16_t id;
  const char *name;
} pci_device_id;

typedef struct {
  uint16_t id;
  const char *name;
  const pci_device_id *devices;
} pci_vendor_id;

extern const pci_vendor_id pci_vendor_list[];
extern const char *pci_class_code[];

/* Please read http://wiki.osdev.org/PCI */

#define PCI_BUS_MAX_NUM 256
#define PCI_DEV_MAX_NUM 32
#define PCI_FUN_MAX_NUM 8

#define PCI_BAR_MEMORY 0
#define PCI_BAR_IO 1
#define PCI_BAR_64BIT 4
#define PCI_BAR_PREFETCHABLE 8

#define PCI_BAR_IO_MASK 3
#define PCI_BAR_MEMORY_MASK 15

#define PCI_BAR_MAX 6

#define PCIR_DEVICEID 0x00
#define PCIR_VENDORID 0x02
#define PCIR_STATUS 0x04
#define PCIR_COMMAND 0x06
#define PCIM_CMD_PORTEN 0x0001
#define PCIM_CMD_MEMEN 0x0002
#define PCIM_CMD_BUSMASTEREN 0x0004
#define PCIR_CLASSCODE 0x08
#define PCIR_HEADERTYPE 0x0d
#define PCIH_HDR_MF 0x80
#define PCIH_HDR_TYPE 0x7f
#define PCIR_IRQPIN 0x3e
#define PCIR_IRQLINE 0x3f
#define PCIR_BAR(i) (0x10 + (i)*4)

typedef struct pci_addr {
  uint8_t bus;
  uint8_t device;
  uint8_t function;
} pci_addr_t;

/* PCI configuration space can be accessed in various ways depending on PCI bus
 * controller and platform architecture. For instance classic PC architecture
 * uses `in` and `out` IA-32 instructions. On MIPS Malta one has to use two
 * memory mapped registers (address and data). */
typedef uint32_t (*pci_read_config_t)(device_t *device, unsigned reg,
                                      unsigned size);
typedef void (*pci_write_config_t)(device_t *device, unsigned reg,
                                   unsigned size, uint32_t value);
typedef void (*pci_enable_busmaster_t)(device_t *device);

typedef struct pci_bus_methods {
  pci_read_config_t read_config;
  pci_write_config_t write_config;
  pci_enable_busmaster_t enable_busmaster;
} pci_bus_methods_t;

typedef struct pci_bar {
  device_t *owner; /* pci device owner of this bar */
  size_t size;     /* identified size of this bar */
  int rid;         /* BAR number in [0,PCI_BAR_MAX-1] */
  unsigned type;   /* RT_IOPORTS or RT_MEMORY */
  unsigned flags;  /* nothing or RF_PREFETACHBLE */
} pci_bar_t;

typedef struct pci_device {
  pci_addr_t addr;

  uint16_t device_id;
  uint16_t vendor_id;
  uint8_t class_code;
  uint8_t pin, irq;

  pci_bar_t bar[PCI_BAR_MAX];
} pci_device_t;

#define PCI_BUS_METHODS(dev)                                                   \
  (*(pci_bus_methods_t *)(dev)->driver->interfaces[DIF_PCI_BUS])

static inline uint32_t pci_read_config(device_t *dev, unsigned reg,
                                       unsigned size) {
  return PCI_BUS_METHODS(dev->parent).read_config(dev, reg, size);
}

#define pci_read_config_1(d, r) pci_read_config((d), (r), 1)
#define pci_read_config_2(d, r) pci_read_config((d), (r), 2)
#define pci_read_config_4(d, r) pci_read_config((d), (r), 4)

static inline void pci_write_config(device_t *dev, unsigned reg, unsigned size,
                                    uint32_t value) {
  PCI_BUS_METHODS(dev->parent).write_config(dev, reg, size, value);
}

#define pci_write_config_1(d, r, v) pci_write_config((d), (r), 1, (v))
#define pci_write_config_2(d, r, v) pci_write_config((d), (r), 2, (v))
#define pci_write_config_4(d, r, v) pci_write_config((d), (r), 4, (v))

static inline void pci_enable_busmaster(device_t *dev) {
  PCI_BUS_METHODS(dev->parent).enable_busmaster(dev);
}

void pci_bus_enumerate(device_t *pcib);
void pci_bus_dump(device_t *pcib);

static inline pci_device_t *pci_device_of(device_t *device) {
  return (device->bus == DEV_BUS_PCI) ? device->instance : NULL;
}

static inline bool pci_device_match(pci_device_t *pcid, uint16_t vendor_id,
                                    uint16_t device_id) {
  return (pcid != NULL) && (pcid->vendor_id == vendor_id) &&
         (pcid->device_id == device_id);
}

#endif /* !_SYS_PCI_H_ */
