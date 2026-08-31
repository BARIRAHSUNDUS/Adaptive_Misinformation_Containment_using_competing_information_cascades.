# Project Assumptions

The following assumptions are made for the simulation of misinformation and
fact-check information propagation:

1. **Graph Representation**  
   The social network is represented as a graph, where users are nodes and
   their connections are edges.

2. **Misinformation Source**  
   Misinformation starts from a predefined user/node and propagates through
   the user's connections.

3. **Fixed Spread Probability**  
   The probability of misinformation spreading through a connection is
   predefined and remains constant throughout the simulation.

4. **Limited Moderator Budget**  
   The moderator has a fixed and limited number of fact-checking
   interventions.

5. **Effect of Fact-Checking**  
   A user who receives a fact-check intervention is considered corrected and
   cannot spread the misinformation further.

6. **Discrete Simulation Steps**  
   The simulation proceeds in discrete time steps, allowing the network state
   to be observed after each step of information propagation.

7. **Static Network Structure**  
   Connections between users remain unchanged throughout the simulation.
   Only the information state of users can change.

8. **Dynamic Intervention Selection**  
   The best intervention may change after each intervention depending on the
   current state of the network. Therefore, intervention decisions are
   recalculated during the simulation.

9. **Fair Strategy Comparison**  
   Random, Highest-Degree, and Marginal-Gain strategies are evaluated under
   the same network, misinformation source, simulation conditions, and number
   of available interventions.
