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
async def cocotb_dataflow_split(dut):
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
	Cc = channel.Channel("Cc", dut.Cc_valid, dut.Cc_ready, dut.Cc_data, dtype=dtype)
	L = channel.Channel("L", dut.L_valid, dut.L_ready, dut.L_data, dtype=dtype)
	R0 = channel.Channel("R0", dut.R0_valid, dut.R0_ready, dut.R0_data, dtype=dtype)
	R1 = channel.Channel("R1", dut.R1_valid, dut.R1_ready, dut.R1_data, dtype=dtype)

	# Environment
	srcCc = channel.Source(Cc, values=Cc_tokens, log=log)
	srcL = channel.Source(L, values=L_tokens, log=log)
	sinkR0 = channel.Sink(R0, values=R0_tokens, log=log)
	sinkR1 = channel.Sink(R1, values=R1_tokens, log=log)

	model = dataflow.Split(Cc, L, R0, R1, log=log)

	for _ in range(10000):
		srcCc.cycle()
		srcL.cycle()

		model.cycle()

		sinkR0.cycle()
		sinkR1.cycle()

		await RisingEdge(dut.clk)

	log.done()

def test_dataflow_split_expected():
	tb = test.Bench("dataflow_split_expected", "cocotb_dataflow_split")
	tb.source("rtl/dataflow_split_expected.v")
	tb.run()

def test_dataflow_split():
	tb = test.Bench("dataflow_split", "cocotb_dataflow_split")
	tb.source("rtl/dataflow_split.v")
	tb.run()
