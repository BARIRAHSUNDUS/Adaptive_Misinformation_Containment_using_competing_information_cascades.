# Module 2 – Cascade Model

## Overview

The Cascade Model is responsible for simulating the spread of misinformation through the social network.

It uses the social graph created by Module 1 and the current state of each user to simulate how misinformation propagates from one user to their connected neighbours.

## Responsibilities

This module is responsible for:

- Simulating misinformation propagation.
- Maintaining the propagation state of users.
- Applying the configured misinformation spreading probability.
- Managing the current propagation frontier.
- Ensuring corrected users cannot become misinformed again.
- Updating user states after each propagation step.

## User States

The simulation uses three user states:

- **Susceptible** – The user has not been affected by misinformation.
- **Misinformed** – The user is currently affected by misinformation and can spread it.
- **Corrected** – The user has been fact-checked and cannot become misinformed again.

## Propagation Model

Misinformation spreads through the edges of the social graph with a predefined probability.

The propagation follows a synchronous, frontier-based approach.

For example:

```text
Tick 1:
        A
       / \
      B   C

A = Misinformed
B, C = Susceptible

If A successfully spreads misinformation:

Tick 2:
        A
       / \
      B   C

B, C = Misinformed

Tick 3:
B and C attempt to spread misinformation
