import cocotb
from cocotb.triggers import RisingEdge
from cocotb.clock import Clock

import numpy as np
import matplotlib.pyplot as plt

import pytest
import random

from puffs import test
from puffs import fixed
from puffs import channel

from models import dataflow

@cocotb.test()
async def cocotb_dataflow_buffer(dut):
	params = test.Params()
	log = test.Logger()

	# Seed logging
	seed = random.randint(-1_000_000_000, 1_000_000_000)
	log.info(f"seed={seed}")
	random.seed(seed)

	# Use pure integer dtype
	dtype = channel.Int()

	# Random input token generators (unsigned WIDTH-bit range)
	L_tokens = channel.RandomInt((0, 256), rate=0.8)
	R_tokens = channel.Dataless(rate=0.8)

	# Start clock
	cocotb.start_soon(Clock(dut.clk, 10, units="ns").start())

	# Reset
	dut.reset.value = 1
	await RisingEdge(dut.clk)
	await RisingEdge(dut.clk)
	dut.reset.value = 0

	# Wrap DUT ports with Channels
	L = channel.Channel("L", dut.L_valid, dut.L_ready, dut.L_data, dtype=dtype)
	R = channel.Channel("R", dut.R_valid, dut.R_ready, dut.R_data, dtype=dtype)

	# Environment
	srcL = channel.Source(L, values=L_tokens, log=log)
	sinkR = channel.Sink(R, values=R_tokens, log=log)

	model = dataflow.Buffer(L, R, log=log)

	for _ in range(10000):
		srcL.cycle()

		model.cycle()

		sinkR.cycle()

		await RisingEdge(dut.clk)

	log.done()

def test_dataflow_buffer_expected():
	tb = test.Bench("dataflow_buffer_expected", "cocotb_dataflow_buffer")
	tb.source("rtl/dataflow_buffer_expected.v")
	tb.run()

#def test_dataflow_buffer():
#	tb = test.Bench("dataflow_buffer", "cocotb_dataflow_buffer")
#	tb.source("rtl/dataflow_buffer.v")
#	tb.run()
