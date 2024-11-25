RX {

	ps2_clk_od = 1
	ps2_dat_od = 1
	pbit = 1
	while (!ps2_clk_negedge);

	for (i = 0; i != 11; i++) {
		
		// wait sampling instant
		for (tim_cnt = 0; tim_cnt != 15 us; tim_cnt++);
		
		// detect error or process data bit
		if (i == 0) {
			if (ps2_dat_sync != 0)
				FRAME_ERROR
		} else if (i == 9) {
			if (ps2_dat_sync != pbit)
				PARITY_ERROR
		} else if (i == 10) {
			if (ps2_dat_sync != 1)
				FRAME_ERROR
		} else {
			shreg >>= 1
			shreg[MSB] = ps2_dat_sync
			pbit ^= ps2_dat_sync
		}

		// wait for next negedge
		if (i != 10) {
			while (!ps2_clk_negedge) {
				if (tim_cnt == 110 us)
					CLK_TIMEOUT
				tim_cnt++
			}
		}
	}

	valid = 1
	rx_data = shreg

}