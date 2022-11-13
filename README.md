# Lorg

Lorg is a hierarchical data manager. You define your data and their associated
units in a text file, and Lorg automatically calculates the missing units.

## How to use Lorg

### Example

It is easier to explain and understand how to use Lorg with an example.

For example you have a new home and you need to renovate some rooms. You want
to know how long it will take to renovate and how much it would cost in total.

Let say you need to renovate the `Living room` and the `Bathroom`. You know how
many `Days` and the `Cost` for the `Living room` room renovation, you only know
the `Cost` for the `Bathroom`.

Let's define your data in a file called `house.lorg`:

```lorg
# House
## First floor
### Living room
$ Days: 2
$ Cost: 500
## Second floor
### Bathroom
$ Cost: 1500
I have no idea how long it will take!
```

Now use Lorg to automatically calculate the total:

```
lorg house.lorg
```

Lorg will print the result:

```
# House
  $ Cost: 2000 [Calculated]
  $ Days: 2 [Calculated]
  ## First floor
    $ Cost: 500 [Calculated]
    $ Days: 2 [Calculated]
    ### Living room
      $ Cost: 500
      $ Days: 2
  ## Second floor
    $ Cost: 1500 [Calculated]
    $ Days: 0 [Calculated]
    ### Bathroom
      $ Cost: 1500
      $ Days: 0 [Calculated]
```

You now know that it will takes at least `2` days and it will cost you `2000`
to renovate your new home.

### The syntax

By convention, we write into files with a `.lorg` extension. In our example, we
write everything into a file named `house.lorg`.

Each component is called a **node**. A node can contain other nodes and
**units**.

#### Nodes

If you know Markdown, you already know how to structure your data.

A node is defined in one line starting by one or multiple `#` then the node
title. The number of `#` defines the node level, in other words it defines if
the node is the child of a previously defined node.

To take the previous example, the `House` has a `First floor` and a `Second
floor`. The `First floor` contains a `Living room` and the `Second floor`
contains a `Bathroom`. In Lorg, it is written this way:

```lorg
# House
## First floor
### Living room
## Second floor
### Bathroom
```

#### Units

A unit is defined in one line starting with one `$` then the unit name then `:`
then the unit value. Unit values can only be integers or decimal-point numbers.

In our example, we know that it takes 2 days and it costs 500 to renovate the
living room. We also know it costs 1500 to renovate the bathroom. We define
units for those nodes.

```lorg
# House
## First floor
### Living room
$ Days: 2
$ Cost: 500
## Second floor
### Bathroom
$ Cost: 1500
```

### Comments

All lines that are not node definitions nor unit definitions are comments. They
are ignored by Lorg.

#### Usage

Lorg will calculate for us the unit values for the other nodes. Note that for
the moment it only **sums** the values.

Lorg contains some options. Here we print the result in a pretty format.

```
lorg --prettify house.lorg
```

It returns this result

```
House
│ $ Cost: 2000 [Calculated]
│ $ Days: 2 [Calculated]
├── First floor
│   │ $ Cost: 500 [Calculated]
│   │ $ Days: 2 [Calculated]
│   └── Living room
│         $ Cost: 500
│         $ Days: 2
└── Second floor
    │ $ Cost: 1500 [Calculated]
    │ $ Days: 0 [Calculated]
    └── Bathroom
          $ Cost: 1500
          $ Days: 0 [Calculated]
```

## Install Lorg

### Dependencies

Lorg was written in C++17. You need `cmake`, `g++` and `make` if you want to
use the build system included in the project.

To install the dependencies on a Debian base system (Ubuntu, Linux Mint...):

```
sudo apt install cmake g++
```

Alternatively, Lorg was developed to be easy to build, so there are no special
dependencies. If you want to build it and install it manually, just use your
favorite C++ compiler and compile all the files. For example with `gcc`:

```
gcc src/* -lstdc++ -o lorg
```

### Build

After installing the dependencies, use the `Makefile` included in the project
to build the `lorg` executable.

```
make
```

### Install and uninstall

You can modify `config.mk` if you want to customize the installation process.
To install Lorg, use this command:

```
sudo make install
```

If you want to uninstall:

```
sudo make uninstall
```
