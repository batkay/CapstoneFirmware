#include <zephyr/kernel.h>
// #include <zephyr/logging/log.h>
// // LOG_MODULE_REGISTER(dmic);

#include <zephyr/audio/dmic.h>
#include <ei_wrapper.h>

#include "audio.h"
//static const struct device *const dmic_dev = DEVICE_DT_GET(DT_NODELABEL(dmic_dev));

#define MAX_SAMPLE_RATE  16000
#define SAMPLE_BIT_WIDTH 16
#define BYTES_PER_SAMPLE sizeof(int16_t)
/* Milliseconds to wait for a block to be read. */
#define READ_TIMEOUT     1000

/* Size of a block for 100 ms of audio data. */
#define BLOCK_SIZE(_sample_rate, _number_of_channels) \
	(BYTES_PER_SAMPLE * (_sample_rate / 10) * _number_of_channels)

/* Driver will allocate blocks from this slab to receive audio data into them.
 * Application, after getting a given block from the driver and processing its
 * data, needs to free that block.
 */
#define MAX_BLOCK_SIZE   BLOCK_SIZE(MAX_SAMPLE_RATE, 2)
#define BLOCK_COUNT      4
K_MEM_SLAB_DEFINE_STATIC(mem_slab, MAX_BLOCK_SIZE, BLOCK_COUNT, 4);

#define FRAME_ADD_INTERVAL_MS	100


static void result_ready_cb(int err)
{
	if (err) {
		printk("Result ready callback returned error (err: %d)\n", err);
		return;
	}

	const char *label;
	float value;
	float anomaly;

	printk("\nClassification results\n");
	printk("======================\n");

	while (true) {
		err = ei_wrapper_get_next_classification_result(&label, &value, NULL);

		if (err) {
			if (err == -ENOENT) {
				err = 0;
			}
			break;
		}

		printk("Value: %.2f\tLabel: %s\n", (double)value, label);
	}

	if (err) {
		printk("Cannot get classification results (err: %d)", err);
	} else {
		if (ei_wrapper_classifier_has_anomaly()) {
			err = ei_wrapper_get_anomaly(&anomaly);
			if (err) {
				printk("Cannot get anomaly (err: %d)\n", err);
			} else {
				printk("Anomaly: %.2f\n", (double)anomaly);
			}
		}
	}

	err = ei_wrapper_start_prediction(0, 1);
	if (err) {
		printk("Cannot restart prediction (err: %d)\n", err);
	} else {
		printk("Prediction restarted...\n");
	}

	
}

static int do_pdm_transfer(const struct device *dmic_dev,
			   struct dmic_cfg *cfg,
			   size_t block_count)
{
	int ret;

	// LOG_INF("PCM output rate: %u, channels: %u", cfg->streams[0].pcm_rate, cfg->channel.req_num_chan);

	ret = dmic_configure(dmic_dev, cfg);
	if (ret < 0) {
		// LOG_ERR("Failed to configure the driver: %d", ret);
		return ret;
	}

	ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_START);
	if (ret < 0) {
		// LOG_ERR("START trigger failed: %d", ret);
		return ret;
	}

	for (int i = 0; i < block_count; ++i) {
		void *buffer;
		uint32_t size;

		ret = dmic_read(dmic_dev, 0, &buffer, &size, READ_TIMEOUT);
		if (ret < 0) {
			// LOG_ERR("%d - read failed: %d", i, ret);
			return ret;
		}

		// LOG_INF("%d - got buffer %p of %u bytes", i, buffer, size);

		k_mem_slab_free(&mem_slab, buffer);
	}

	ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
	if (ret < 0) {
		// LOG_ERR("STOP trigger failed: %d", ret);
		return ret;
	}

	return ret;
}

int audioSample() {
	int ret;
	const struct device *const dmic_dev = DEVICE_DT_GET(DT_NODELABEL(dmic));

	// LOG_INF("DMIC sample");

	if (!device_is_ready(dmic_dev)) {
		// LOG_ERR("%s is not ready", dmic_dev->name);
		return 0;
	}

	struct pcm_stream_cfg stream = {
		.pcm_width = SAMPLE_BIT_WIDTH,
		.mem_slab  = &mem_slab,
	};
	struct dmic_cfg cfg = {
		.io = {
			/* These fields can be used to limit the PDM clock
			 * configurations that the driver is allowed to use
			 * to those supported by the microphone.
			 */
			.min_pdm_clk_freq = 1000000,
			.max_pdm_clk_freq = 3500000,
			.min_pdm_clk_dc   = 40,
			.max_pdm_clk_dc   = 60,
		},
		.streams = &stream,
		.channel = {
			.req_num_streams = 1,
		},
	};

	cfg.channel.req_num_chan = 1;
	cfg.channel.req_chan_map_lo =
		dmic_build_channel_map(0, 0, PDM_CHAN_LEFT);
	cfg.streams[0].pcm_rate = MAX_SAMPLE_RATE;
	cfg.streams[0].block_size =
		BLOCK_SIZE(cfg.streams[0].pcm_rate, cfg.channel.req_num_chan);

	// ret = do_pdm_transfer(dmic_dev, &cfg, 2 * BLOCK_COUNT);

	// LOG_INF("PCM output rate: %u, channels: %u", cfg->streams[0].pcm_rate, cfg->channel.req_num_chan);

	ret = dmic_configure(dmic_dev, &cfg);
	if (ret < 0) {
		// LOG_ERR("Failed to configure the driver: %d", ret);
		return ret;
	}



	int err = ei_wrapper_init(result_ready_cb);

	if (err) {
		printk("Edge Impulse wrapper failed to initialize (err: %d)\n",
		       err);
		return 0;
	};

	printk("Machine learning model sampling frequency: %zu\n",
	       ei_wrapper_get_classifier_frequency());
	printk("Labels assigned by the model:\n");
	for (size_t i = 0; i < ei_wrapper_get_classifier_label_count(); i++) {
		printk("- %s\n", ei_wrapper_get_classifier_label(i));
	}
	printk("\n");

	/* input_data is defined in input_data.h file. */

	err = ei_wrapper_start_prediction(0, 0);
	if (err) {
		printk("Cannot start prediction (err: %d)\n", err);
	} else {
		printk("Prediction started...\n");
	}

	/* Predictions for additional data are triggered in the result ready
	 * callback. The prediction start can be triggered before the input
	 * data is provided. In that case the prediction is started right
	 * after the prediction window is filled with data.
	 */





	ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_START);
	if (ret < 0) {
		// LOG_ERR("START trigger failed: %d", ret);
		return ret;
	}

	while (true) {
		void *buffer;
		uint32_t size;

		ret = dmic_read(dmic_dev, 0, &buffer, &size, READ_TIMEOUT);
		if (ret < 0) {
			// LOG_ERR("%d - read failed: %d", i, ret);
			return ret;
		}

		// LOG_INF("%d - got buffer %p of %u bytes", i, buffer, size);

		err = ei_wrapper_add_data(buffer, size);

		k_mem_slab_free(&mem_slab, buffer);
	}
	return 0;
}