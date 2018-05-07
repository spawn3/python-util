package main

import (
    "fmt"
    "util"
    "wbuf"
    "simplemath"
)

func test1() {
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

func testOOP() {
    b := simplemath.Base{"Base"}
    f := &simplemath.Foo{10, b}

    fmt.Printf("Name %s\n", f.Name)
    fmt.Printf("Foo() %s\n", f.Foo())
    fmt.Printf("Bar() %s\n", f.Bar())
}

func testSlice() {
    xs := []int{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}

    for k, v := range xs {
        fmt.Printf("slice %d %d\n", k, v)
    }
}

func testMap() {
    xs := map[string]int{"A": 1, "B": 2, "C": 3}

    for k, v := range xs {
        fmt.Printf("map %s %d\n", k, v)
    }
}

func main() {
    testSlice()
    testMap()
    testOOP()
}
