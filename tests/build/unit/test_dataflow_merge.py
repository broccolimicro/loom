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
async def cocotb_dataflow_merge(dut):
	params = test.Params()
	log = test.Logger()

	# Seed logging
	seed = random.randint(-1_000_000_000, 1_000_000_000)
	log.info(f"seed={seed}")
	random.seed(seed)

	# Use pure integer dtype
	dtype = channel.Int()

	# Random input token generators (unsigned WIDTH-bit range)
	Cc_tokens = channel.RandomInt((0, 2), rate=0.8)
	L0_tokens = channel.RandomInt((0, 256), rate=0.8)
	L1_tokens = channel.RandomInt((0, 256), rate=0.8)
	R_tokens = channel.Dataless(rate=0.8)

	# Start clock
	cocotb.start_soon(Clock(dut.clk, 10, units="ns").start())

	# Reset
	dut.reset.value = 1
	await RisingEdge(dut.clk)
	await RisingEdge(dut.clk)
	dut.reset.value = 0

	# Wrap DUT ports with Channels
	Cc = channel.Channel("Cc", dut.Cc_valid, dut.Cc_ready, dut.Cc_data, dtype=dtype)
	L0 = channel.Channel("L0", dut.L0_valid, dut.L0_ready, dut.L0_data, dtype=dtype)
	L1 = channel.Channel("L1", dut.L1_valid, dut.L1_ready, dut.L1_data, dtype=dtype)
	R = channel.Channel("R", dut.R_valid, dut.R_ready, dut.R_data, dtype=dtype)

	# Environment
	srcCc = channel.Source(Cc, values=Cc_tokens, log=log)
	srcL0 = channel.Source(L0, values=L0_tokens, log=log)
	srcL1 = channel.Source(L1, values=L1_tokens, log=log)
	sinkR = channel.Sink(R, values=R_tokens, log=log)

	model = dataflow.Merge(Cc, L0, L1, R, log=log)

	for _ in range(10000):
		srcCc.cycle()
		srcL0.cycle()
		srcL1.cycle()

		model.cycle()

		sinkR.cycle()

		await RisingEdge(dut.clk)

	log.done()

def test_dataflow_merge_expected():
	tb = test.Bench("dataflow_merge_expected", "cocotb_dataflow_merge")
	tb.source("rtl/dataflow_merge_expected.v")
	tb.run()

def test_dataflow_merge():
	tb = test.Bench("dataflow_merge", "cocotb_dataflow_merge")
	tb.source("rtl/dataflow_merge.v")
	tb.run()
