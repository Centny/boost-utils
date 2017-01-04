package main

import (
	"bufio"
	"fmt"
	"net"
)

func main() {
	p := make([]byte, 2048)
	conn, err := net.Dial("udp", "127.0.0.1:14222")
	if err != nil {
		fmt.Printf("Some error %v", err)
		return
	}
	conn.Write([]byte{8, 0, 3, 'a', 'b', 'c'})
	_, err = bufio.NewReader(conn).Read(p)
	if err == nil {
		fmt.Printf("%s\n", p)
	} else {
		fmt.Printf("Some error %v\n", err)
	}
	conn.Close()
}
