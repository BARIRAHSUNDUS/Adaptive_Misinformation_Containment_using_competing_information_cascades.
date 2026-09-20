# Module 4 – Simulation Engine

## Overview

The Simulation Engine is responsible for coordinating the complete misinformation containment simulation.

It connects the Social Graph, Cascade Model, and Intervention Strategy modules and controls the execution of the simulation from start to finish.

## Responsibilities

This module is responsible for:

- Initializing the simulation using the configured parameters.
- Managing the simulation ticks.
- Controlling misinformation propagation.
- Scheduling and applying moderator interventions.
- Managing the remaining intervention budget.
- Coordinating the Cascade Model and Intervention Strategy.
- Running experiments for different intervention strategies.
- Collecting and calculating simulation results.
- Measuring execution time.

## Simulation Flow

The Simulation Engine follows this general process:

```text
Simulation Configuration
          ↓
Create Social Graph
          ↓
Initialize User States
          ↓
Select Misinformation Source
          ↓
Run Simulation
          ↓
Intervention Strategy
          ↓
Apply Fact-Check Intervention
          ↓
Misinformation Propagation
          ↓
Update User States
          ↓
Repeat Until Termination
          ↓
Calculate Results
