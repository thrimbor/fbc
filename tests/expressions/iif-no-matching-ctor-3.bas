' TEST_MODE : COMPILE_ONLY_FAIL

type UDT
	i as integer
	declare constructor( as integer )
end type

constructor UDT( i as integer )
end constructor

dim as integer c
dim as UDT x1 = UDT( 0 )

'' We can construct the temp var from the true expression but not from the
'' false one, so it must be an error
x1 = iif( c, UDT( 0 ), x1 )
