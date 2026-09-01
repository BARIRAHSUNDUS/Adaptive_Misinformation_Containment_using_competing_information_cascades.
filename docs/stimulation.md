# Simulation

## Simulation Flow

The simulation runs in discrete rounds.

1. Initialize the social network.
2. Select the misinformation source.
3. Simulate misinformation propagation.
4. Select an intervention user according to the chosen strategy.
5. Apply the fact-check intervention.
6. Update the network state.
7. Continue until the moderator budget is exhausted or the simulation ends.

## Inputs

- Number of users
- Network connections
- Misinformation source
- Spread probability
- Number of rounds
- Moderator budget

## Outputs

- Misinformation reach
- Misinformation prevented
- Fact-check reach
- Execution time
