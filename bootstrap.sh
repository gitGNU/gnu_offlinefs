#!/bin/bash
libtoolize && aclocal && autoheader && automake -a --foreign && autoconf
