# Lorg

_If you are reading this on GitHub or other Git repository or service, then you
are looking at a mirror. The official repository is [this Fossil
repository](https://dev.lorg.software/lorg)._

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

#### Comments

All lines that are not node definitions nor unit definitions are comments. They
are ignored by Lorg.

### Usage

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

Lorg is written in C++17. You need `cmake`, `g++` and `make` if you want to use
the build system included in the project.

To install the dependencies on a Debian base system (Ubuntu, Linux Mint...):

```
sudo apt install cmake g++
```

Alternatively, Lorg is developed to be easy to build, so there are no special
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

## License

This project is licensed under GNU AGPLv3 (GNU Affero General Public License
version 3). The terms and conditions of this license are in the `LICENSE` file.

Note that in the section "2. Basic Permissions.", the license gives you the
right to copy and paste small code snippets for various purposes (share
snippets of the code, use snippets for your own work...) without the need of
covering your own work under GNU AGPLv3.

> This License acknowledges your rights of fair use or other equivalent, as
> provided by copyright law.

In other words, if you integrate in your project some small parts of the code
from this project, you are not forced to license your work under GNU AGPLv3.

## Copyright

```
Lorg - a hierarchical data manager - CLI version
Copyright (C) 2023  Alex Canales

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```
