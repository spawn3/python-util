package main

import (
    "fmt"
    "util"
    "wbuf"
)


func main() {
    pWbuf := new(wbuf.Wbuf)
    pWbuf.Init()

    fmt.Printf("w = %d\n", pWbuf.A)

    var i, j int
    k := 2

    i, j = 0, 1

    fmt.Printf("hello i %d j %d k %d\n", i, j, k)

    var ch chan int
    ch = make(chan int)

    go func(ch chan int) {
        fmt.Printf("go here\n")
        ch <- 1
    }(ch)

    v := <- ch
    fmt.Printf("v = %d\n", v);

    fmt.Printf("add(1, 2) = %d\n", util.Add(1, 2))
    fmt.Printf("add2(1, 2) = %d\n", util.Add2(1, 2))
}
