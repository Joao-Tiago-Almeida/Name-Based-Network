# Name-Based-Network

## Introduction

A name based Network is a network where a node has a set of named objects which also gets named objects own by other nodes. A name is composed by a unique identifier of the node in the network that has it, followed by a subname localy unique attributed by the node. Each node has a cache memory with capacity to store a small amount of objects. The network topology is a tree.

When a user of a origin node wants to find a specific node, that node sends a _intereset_  message with the name of that object. The _intereset_ message is forward in the network in direction of the node that contains the object. The destination node or other intermidiate that has the the object stored in its cache answers to the _intereset_ message with an _object_ message contacining the name and the interest object. The origin node and the intermidiate which forward the _object_ message from the destination to the origin stored that object in its cache.


# Ease of use

The aplication starts by compiling it with the `make` command in the [src](./scr) folder. The next setp is to run the **ndn** executable as

```
./ndn IP TCP
```

where the IP TCP are the local IP and port of each node. To create a network use the command join only without the fouth and fifth arguments, and to join a node to a speacific node (which are already in a network) use all 5 arguments:

```
join <key_of_the_network> <name_of_the_node> <IP_of_the_neighbour> <PORT_of_the_neighbour>
```

After a node is in the network the following User Interaface is shown with the available commands:

![User Interface](https://user-images.githubusercontent.com/39059647/121732835-9bcd1980-caea-11eb-80f6-e2b64726fe5c.png)

I developed this project with [Carolina](https://github.com/carolinafidalgo).

The final grade of this project was 20/20.
