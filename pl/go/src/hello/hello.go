package main

import (
    "fmt"
    "util"
    "wbuf"
    "simplemath"
    "time"
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

func say(s string) {
    for i := 0; i < 5; i++ {
        time.Sleep(100 * time.Millisecond)
        fmt.Println(s)
    }
}

func sum(s []int, c chan int) {
    sum := 0
    for _, v := range s {
        sum += v
    }

    c <- sum
}

func testChan() {
    s := []int{7, 2, 8, -9, 4, 0}

    c := make(chan int)

    go sum(s[:len(s)/2], c)
    go sum(s[len(s)/2:], c)

    x, y := <-c, <- c

    fmt.Println(x, y, x+y)
}

func main() {
    // testSlice()
    // testMap()
    // testOOP()

    // go say("World")
    // say("Hello")

    testChan()
}
