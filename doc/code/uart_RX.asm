frame[1 + NCHAR + PARITY_NONE?0:1 + NSTOP] = {1, ..., 1};
pbit = PARITY_ODD ? 1 : 0;
tim_reload_val = HALF_BIT_TIME;
tim_auto_reload_val = BIT_TIME;

while(!urx_negedge);
tim_reload();

do {
	// wait sampling instant
	while(!tim_tc) {
		// if there are transitions, resync clocks
		if (urx_edge)
			tim_reload();
	}
	// sample 
	frame = {urx_sync, frame[MSB:1]};
	// update parity (start, parity xor'd too)
	if (!PARITY_NONE & (&frame[NSTOP-1:0]))
		pbit ^= urx_sync;

} while (frame[0]);

frame_error = ! &frame[MSB -: NSTOP];
parity_error = PARITY_NONE ? 0 : pbit;
rx_data = frame[1 +: NCHAR];
valid = 1;
