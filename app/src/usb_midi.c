
#include <zephyr/usb/usb_device.h>
#include <zmk/usb_midi.h>
#include <zmk/usb_midi_packet.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static K_SEM_DEFINE(midi_sem, 1, 1);

static int usb_midi_is_available = false;

// TODO do we need this???
/*
 * These macros should be used to place the USB descriptors
 * in predetermined order in the RAM.
 *  #define USBD_CLASS_DESCR_DEFINE(p, instance)                         \
 *          static __in_section(usb, descriptor_##p.1, instance) __used __aligned(1)
 */
USBD_CLASS_DESCR_DEFINE(primary, 1)


struct usb_midi_config usb_midi_config_data = {
	.ac_if = INIT_AC_IF,
	.ac_cs_if = INIT_AC_CS_IF,
	.ms_if = INIT_MS_IF,
	.ms_cs_if = INIT_MS_CS_IF,
	.out_jacks_emb = {
		LISTIFY(USB_MIDI_NUM_OUTPUTS, INIT_OUT_JACK, (, ), 0)},
	.in_jacks_emb = {LISTIFY(USB_MIDI_NUM_INPUTS, INIT_IN_JACK, (, ), USB_MIDI_NUM_OUTPUTS)},
	.element = INIT_ELEMENT,
	.in_ep = INIT_IN_EP,
	.in_cs_ep = {.bLength = sizeof(struct usb_midi_bulk_in_ep_descriptor), .bDescriptorType = USB_DESC_CS_ENDPOINT, .bDescriptorSubtype = 0x01, .bNumEmbMIDIJack = USB_MIDI_NUM_OUTPUTS, .BaAssocJackID = {LISTIFY(USB_MIDI_NUM_OUTPUTS, IDX_WITH_OFFSET, (, ), 1)}},
	.out_ep = INIT_OUT_EP,
	.out_cs_ep = {.bLength = sizeof(struct usb_midi_bulk_out_ep_descriptor), .bDescriptorType = USB_DESC_CS_ENDPOINT, .bDescriptorSubtype = 0x01, .bNumEmbMIDIJack = USB_MIDI_NUM_INPUTS, .BaAssocJackID = {LISTIFY(USB_MIDI_NUM_INPUTS, IDX_WITH_OFFSET, (, ), 1 + USB_MIDI_NUM_OUTPUTS)}}
};


void usb_status_callback(struct usb_cfg_data *cfg,
						 enum usb_dc_status_code cb_status,
						 const uint8_t *param)
{
	switch (cb_status)
	{
	/** USB error reported by the controller */
	case USB_DC_ERROR:
		LOG_DBG("USB_DC_ERROR");
		break;
	/** USB reset */
	case USB_DC_RESET:
		LOG_DBG("USB_DC_RESET");
		break;
	/** USB connection established, hardware enumeration is completed */
	case USB_DC_CONNECTED:
		LOG_DBG("USB_DC_CONNECTED");
		break;
	/** USB configuration done */
	case USB_DC_CONFIGURED:
		LOG_DBG("USB_DC_CONFIGURED");
    LOG_INF("USB MIDI device is available");
		usb_midi_is_available = true;
		break;
	/** USB connection lost */
	case USB_DC_DISCONNECTED:
		LOG_DBG("USB_DC_DISCONNECTED");
		break;
	/** USB connection suspended by the HOST */
	case USB_DC_SUSPEND:
		LOG_DBG("USB_DC_SUSPEND");
    LOG_INF("USB MIDI device is unavailable");
		usb_midi_is_available = false;
		break;
	/** USB connection resumed by the HOST */
	case USB_DC_RESUME:
		LOG_DBG("USB_DC_RESUME");
		break;
	/** USB interface selected */
	case USB_DC_INTERFACE:
		LOG_DBG("USB_DC_INTERFACE");
		break;
	/** Set Feature ENDPOINT_HALT received */
	case USB_DC_SET_HALT:
		LOG_DBG("USB_DC_SET_HALT");
		break;
	/** Clear Feature ENDPOINT_HALT received */
	case USB_DC_CLEAR_HALT:
		LOG_DBG("USB_DC_CLEAR_HALT");
		break;
	/** Start of Frame received */
	case USB_DC_SOF:
		LOG_DBG("USB_DC_SOF");
		break;
	/** Initial USB connection status */
	case USB_DC_UNKNOWN:
		LOG_DBG("USB_DC_UNKNOWN");
		break;
	}
}



static void midi_out_ep_cb(uint8_t ep, enum usb_dc_ep_cb_status_code
										   ep_status)
{
  LOG_DBG("midi_out_ep_cb is not implemented");
}

static void midi_in_ep_cb(uint8_t ep, enum usb_dc_ep_cb_status_code ep_status)
{
  LOG_DBG("midi_in_ep_cb is not implemented");
}

static struct usb_ep_cfg_data midi_ep_cfg[] = {
	{
		.ep_cb = midi_in_ep_cb,
		.ep_addr = USB_MIDI_EP_IN,
	},
	{
		.ep_cb = midi_out_ep_cb,
		.ep_addr = USB_MIDI_EP_OUT,
	}};


//TODO this is the macro that sets up the usb device for midi
// will this fight the one defined by zephyr usb_hid.c
/*
 * This macro should be used to place the struct usb_cfg_data
 * inside usb data section in the RAM.
 * #define USBD_DEFINE_CFG_DATA(name)               \
 * static STRUCT_SECTION_ITERABLE(usb_cfg_data, name)
 */

USBD_DEFINE_CFG_DATA(usb_midi_config) = {
	.usb_device_description = NULL,
	.interface_config = NULL,
	.interface_descriptor = &usb_midi_config_data.ac_if,
	.cb_usb_status = usb_status_callback,
	.interface = {
		.class_handler = NULL,
		.custom_handler = NULL,
		.vendor_handler = NULL,
	},
	.num_endpoints = ARRAY_SIZE(midi_ep_cfg),
	.endpoint = midi_ep_cfg,
};


int zmk_usb_send_midi_report(struct zmk_midi_key_report_body* body){
  //TODO implement like zmk_usb_hid_send_mouse_report() and related?
  LOG_ERR("not implemented");

  //TODO take the zmk_midi_key_report_body pointer, read out the set keys from the bitmap and
  // call zmk_usb_hid_send on them
  return 1;
}

static int zmk_usb_midi_send(uint8_t *cable_number, uint8_t *midi_bytes, size_t len) {

  // prepare the packet
	struct usb_midi_packet_t packet;
	enum usb_midi_error_t error = usb_midi_packet_from_midi_bytes(midi_bytes, cable_number, &packet);
	if (error != USB_MIDI_SUCCESS)
    {
      LOG_ERR("Building packet from MIDI bytes %02x %02x %02x failed with error %d", midi_bytes[0], midi_bytes[1], midi_bytes[2], error);
      return -EINVAL;
    }

  // ensure usb is ready
  switch (zmk_usb_get_status()) {
  case USB_DC_SUSPEND:
    return usb_wakeup_request();
  case USB_DC_ERROR:
  case USB_DC_RESET:
  case USB_DC_DISCONNECTED:
  case USB_DC_UNKNOWN:
    return -ENODEV;
  default:
    k_sem_take(&midi_sem, K_MSEC(30));
    uint32_t num_written_bytes = 0;
    usb_write(USB_MIDI_EP_IN, packet.bytes, 4, &num_written_bytes);


    // TODO error if num_written_bytes != 4, make sure to release sem on error like usb_hid.c

    // TODO usb_hid.c holds the sem until its in_ready_cb is hit. do we have something like this?
    // usb status seems to be different, perhaps that is using hid status?
    // anyway, for now just release the sem right after we transmit

    k_sem_give(&midi_sem);

    return 0;
  }
}