#!/bin/bash
aclocal && autoheader && automake -a --foreign && autoconf
