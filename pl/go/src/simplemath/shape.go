package simplemath

// import "math"

type Shape interface {
    Area() float64
}

type Circle struct {
    r int;
}

func (c *Circle) Area() float64 {
    return 3.1415926 * float64(c.r) * float64(c.r)
}

type Base struct {
    Name string
}

func (base *Base)Bar() string {
    return base.Name + "::Bar"
}

func (base *Base)Foo() string {
    return base.Name + "::Foo"
}

type Foo struct {
    Age int
    Base
}
