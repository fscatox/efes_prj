TX {
	
	// init
	shreg = tx_data
	pbit = 1

	// request-to-send configuration
	ps2_clk_od = 0
	ps2_dat_od = 0
	for (tim_cnt = 0; tim_cnt != 100 us; tim_cnt++)

	// generate start bit
	ps2_clk_od = 1

	for (i=1; i != 12; i++) {

		// wait for the next data-generation edge
		while (!ps2_clk_negedge) {
			if (i == 1 && tim_cnt == 15 ms)
				RQST_TIMEOUT
			else if (i != 1 && tim_cnt == 110 us)
				CLK_TIMEOUT
			tim_cnt++
		}

		tim_cnt = 0

		if (i == 11) {
			ps2_dat_od = 1
			// wait sampling instant
			for (;tim_cnt != 15 us; tim_cnt++);

			// detect missing ack
			if (ps2_dat_sync != 0)
				FRAME_ERROR
		}
		else if (i == 10) {
			ps2_dat_od = 1
		} 
		else if (i == 9) {
			ps2_dat_od = pbit
		} 
		else {
			ps2_dat_od = shreg[0]
			pbit ^= shreg[0]
			shreg >>= 1
		}

	}
	
	valid = 1
	ps2_clk_od = 0


}
