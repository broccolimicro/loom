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
async def cocotb_dataflow_source(dut):
	params = test.Params()
	log = test.Logger()

	# Seed logging
	seed = random.randint(-1_000_000_000, 1_000_000_000)
	log.info(f"seed={seed}")
	random.seed(seed)

	# Use pure integer dtype
	dtype = channel.Int()

	# Random input token generators (unsigned WIDTH-bit range)
	R_tokens = channel.Dataless(rate=0.8)

	# Start clock
	cocotb.start_soon(Clock(dut.clk, 10, units="ns").start())

	# Reset
	dut.reset.value = 1
	await RisingEdge(dut.clk)
	await RisingEdge(dut.clk)
	dut.reset.value = 0

	# Wrap DUT ports with Channels
	R = channel.Channel("R", dut.R_valid, dut.R_ready, dut.R_data, dtype=dtype)

	# Environment
	sinkR = channel.Sink(R, values=R_tokens, log=log)

	model = dataflow.Source(R, log=log)

	for _ in range(10000):
		model.cycle()

		sinkR.cycle()

		await RisingEdge(dut.clk)

	log.done()

def test_dataflow_source_expected():
	tb = test.Bench("dataflow_source_expected", "cocotb_dataflow_source")
	tb.source("rtl/dataflow_source_expected.v")
	tb.run()

#def test_dataflow_source():
#	tb = test.Bench("dataflow_source", "cocotb_dataflow_source")
#	tb.source("rtl/dataflow_source.v")
#	tb.run()
