package simplemath

import "testing"

func TestCircleArea1(t *testing.T) {
    var shape = &Circle{1}
    var r = shape.Area()
    if r != 3.1415926 {
        t.Errorf("Sqrt(16) failed. Got %f expected 3.", r)
    }
}
