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
async def cocotb_dataflow_copy(dut):
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
	R0_tokens = channel.Dataless(rate=0.8)
	R1_tokens = channel.Dataless(rate=0.8)

	# Start clock
	cocotb.start_soon(Clock(dut.clk, 10, units="ns").start())

	# Reset
	dut.reset.value = 1
	await RisingEdge(dut.clk)
	await RisingEdge(dut.clk)
	dut.reset.value = 0

	# Wrap DUT ports with Channels
	L = channel.Channel("L", dut.L_valid, dut.L_ready, dut.L_data, dtype=dtype)
	R0 = channel.Channel("R0", dut.R0_valid, dut.R0_ready, dut.R0_data, dtype=dtype)
	R1 = channel.Channel("R1", dut.R1_valid, dut.R1_ready, dut.R1_data, dtype=dtype)

	# Environment
	srcL = channel.Source(L, values=L_tokens, log=log)
	sinkR0 = channel.Sink(R0, values=R0_tokens, log=log)
	sinkR1 = channel.Sink(R1, values=R1_tokens, log=log)

	model = dataflow.Copy(L, R0, R1, log=log)

	for _ in range(10000):
		srcL.cycle()

		model.cycle()

		sinkR0.cycle()
		sinkR1.cycle()

		await RisingEdge(dut.clk)

	log.done()

def test_dataflow_copy_expected():
	tb = test.Bench("dataflow_copy_expected", "cocotb_dataflow_copy")
	tb.source("rtl/dataflow_copy_expected.v")
	tb.run()

def test_dataflow_copy():
	tb = test.Bench("dataflow_copy", "cocotb_dataflow_copy")
	tb.source("rtl/dataflow_copy.v")
	tb.run()
