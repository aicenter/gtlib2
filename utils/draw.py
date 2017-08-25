import example
from graphviz import Digraph
dot = Digraph(comment='Pursuit evasion - EFG')
q = example.initialization()
b = example.graph2()
IS = example.array2()
players = example.play()

colors = ["blue", "yellow", "green", "red", "cyan", "orange", "gold", "lightskyblue", "wheat", "grey", "seagreen", "pink", "limegreen", "aquamarine", "greenyellow"]

colors2 = [
    "antiquewhite1", "antiquewhite2", "antiquewhite3", "antiquewhite4", "aquamarine",
    "aquamarine3", "aquamarine4", "azure2", "azure3", "azure4", "blue", "blue3",
    "blue4", "blueviolet", "brown", "brown1", "brown3", "brown4", "burlywood1",
    "burlywood3", "burlywood4", "cadetblue1", "cadetblue2",	"cadetblue3",
    "cadetblue4", "chartreuse1", "chartreuse3", "chartreuse4", "chocolate1",
    "chocolate3", "chocolate4", "coral1", "coral3", "cornflowerblue", "cornsilk2",
    "cornsilk3", "cornsilk4", "crimson", "cyan", "cyan3", "cyan4", "darkgoldenrod",
    "darkgoldenrod1", "darkgoldenrod2", "darkgoldenrod3", "darkgoldenrod4",
    "darkgreen", "darkkhaki", "darkolivegreen1", "darkolivegreen2", "darkolivegreen3",
    "darkolivegreen4", "darkorange", "darkorange3", "darkorange4", "darkorchid1",
    "darkorchid3", "darksalmon", "darkseagreen1", "darkseagreen3", "darkseagreen4",
    "darkslategray1", "darkslategray3", "darkslategray4", "deeppink1", "deeppink3",
    "deeppink4", "deepskyblue1", "deepskyblue3", "deepskyblue4", "dimgray",
    "dodgerblue1", "dodgerblue3", "firebrick", "firebrick1", "firebrick3", "forestgreen",
    "gainsboro", "gold1", "gold3", "gold4", "goldenrod2", "gray", "green", "green3",
    "green4", "greenyellow", "honeydew3", "honeydew4", "hotpink1", "hotpink3", "hotpink4",
    "indianred1", "indianred3", "indianred4", "indigo", "khaki1", "khaki2", "khaki3",
    "khaki4", "lawngreen", "lemonchiffon2", "lemonchiffon3", "lightblue1","lightblue2",
    "lightblue3", "lightcoral", "lightcyan2", "lightcyan3", "lightgoldenrod",
    "lightgrey", "lightpink1", "lightpink3", "lightpink4","lightsalmon1", "lightsalmon2",
    "lightsalmon3", "lightsalmon4",	"lightseagreen", "lightskyblue2", "lightskyblue3",
    "lightslateblue", "lightslategray", "lightsteelblue2", "lightyellow2", "lightyellow3",
    "limegreen", "magenta1", "magenta3", "maroon", "maroon1", "maroon3", "mediumaquamarine",
    "mediumorchid1", "mediumorchid3", "mediumpurple2", "mediumseagreen", "mediumslateblue",
    "mediumspringgreen", "mediumturquoise", "mediumvioletred", "mistyrose1", "mistyrose3",
    "mistyrose4", "moccasin", "navajowhite3", "navajowhite4", "olivedrab",
    "olivedrab1", "olivedrab3", "orange1", "orange3", "orange4", "orangered1",
    "orangered3", "orchid1", "orchid3", "orchid4", "palegreen1", "palegreen3",
    "palegreen4", "paleturquoise1", "paleturquoise3", "paleturquoise4", "palevioletred1",
    "palevioletred3", "palevioletred4", "peachpuff1", "peachpuff3", "peachpuff4",
    "peru", "pink1", "pink3", "plum1", "plum3", "plum4", "purple1", "purple3", "red1",
    "red3", "red4", "rosybrown", "rosybrown4", "royalblue1", "royalblue3",
    "saddlebrown", "salmon1", "salmon3", "sandybrown", "seashell3", "seashell4", "sienna1", "sienna3",
    "skyblue4", "slateblue1", "slateblue3", "slategray2", "slategray3", "slategrey",
    "snow3", "springgreen1", "springgreen3", "springgreen4", "steelblue1", "steelblue3",
    "steelblue4", "tan", "tan1", "tan3", "tan4", "thistle2", "thistle3", "thistle4",
    "tomato1", "tomato3", "tomato4", "turquoise", "turquoise1", "turquoise3",
    "turquoise4", "violet", "violetred", "violetred1", "wheat1", "wheat3", "yellow1",
    "yellow3", "yellowgreen", "skyblue1", "skyblue3", "seagreen3", "seagreen4", "navy", "seagreen1"]
for i in range(len(q)):
    #if(IS[i] == -1 or (players[i] == 1 and q[i] == 1)):
    if(IS[i] == -1 and players[i] != 2):
        continue
    dot.attr('node', style='filled', color=colors[IS[i]])
    dot.node(str(i),b[i])

for i in range(len(q)-1,-1,-1):
    #if(IS[i] == -1 or (players[i] == 1 and q[i] == 1)):
    if(IS[i] == -1 and players[i] != 2):
        continue
    for j in range(i, -1,-1):
        if(players[i] == 2):
            if(players[j] == 1 and q[j] == q[i]):
                if(q[j] == 1):
                    dot.edge(str(j), str(i), minlen = str((i-j-1)*0.2 + 1))
                else:
                    dot.edge(str(j), str(i))
                break
        if(players[i] == 1):
            if(players[j] == 0 and q[j] == q[i]):
                if(q[j] == 1):
                    dot.edge(str(j), str(i), minlen = str((i-j-1)*0.2 + 1))
                else:
                    dot.edge(str(j), str(i))
                break
        elif(q[j] > q[i]):
            dot.edge(str(j), str(i))
            break
dot.render('tree-oneplayer2.gv', view=True)
