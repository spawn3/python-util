package wbuf

type Chunk struct {
    Data [1024*1024]byte
    PageIdx int
}

type WAL struct {
    // chunks []Chunk
    ChunkNum int

}

type Wbuf struct {
    A int
}

func (w *Wbuf) Init() int {
    w.A = 1;
    return w.A
}
