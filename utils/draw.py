import example
from graphviz import Digraph
dot = Digraph(comment='Pursuit evasion - EFG')
q = example.initialization()
b = example.graph2()
IS = example.array2()
players = example.play()

colors = ["blue", "yellow", "green", "red", "cyan", "orange", "gold", "lightskyblue", "wheat", "grey", "seagreen", "pink", "limegreen", "aquamarine", "greenyellow"]

for i in range(len(q)):
    dot.attr('node', style='filled', color=colors[IS[i]])
    dot.node(str(i),b[i])

for i in range(len(q)-1,-1,-1):
    for j in range(i, -1,-1):
        if(players[i] == 1):
            if(players[j] == 0 and q[j] == q[i]):
                dot.edge(str(j), str(i))
                break
        elif(q[j] > q[i]):
            dot.edge(str(j), str(i))
            break
dot.render('tre.gv', view=True)
