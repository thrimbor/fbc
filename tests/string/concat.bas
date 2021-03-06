# include "fbcu.bi"




namespace fbc_tests.string_.concat

sub concatTest cdecl ()

	dim s as string = "S", cs as const string = s
	dim z as zstring*10 = "Z", cz as const zstring*10 = z
	dim w as wstring*10 = "W", cw as const wstring*10 = w

	dim ps as  string ptr = @s, pcs as const  string ptr = @cs
	dim pz as zstring ptr = @z, pcz as const zstring ptr = @cz
	dim pw as wstring ptr = @w, pcw as const wstring ptr = @cw

	dim as integer success = 0, fail = 0

	#macro assert_equal(a, b)
		if ((a) = (b)) then
			success += 1
		else
			fail += 1
			print (a) & "!=" & (b)
		end if
	#endmacro

	#macro TEST(a, b)
		scope
			'var cat = (a) + (b)
			dim as string cat = (a) + (b)
			cat += " "
			var cmp = ucase( right(#a, 1) + right(#b, 1) + " ")
			CU_ASSERT_EQUAL( cat, cmp )
		end scope
	#endmacro

	#macro TESTP(ca, cb)
		TEST(    ca,     cb)
		TEST(    ca, *p##cb)
		TEST(*p##ca,     cb)
		TEST(*p##ca, *p##cb)
	#endmacro

	#macro TESTC(a, b)
		TESTP(   a,    b)
		TESTP(   a, c##b)
		TESTP(c##a,    b)
		TESTP(c##a, c##b)
	#endmacro

	TESTC(s, s)
	TESTC(s, z)
	TESTC(s, w)

	TESTC(z, s)
	TESTC(z, z)
	TESTC(z, w)

	TESTC(w, s)
	TESTC(w, z)
	TESTC(w, w)

end sub

sub ctor () constructor

	fbcu.add_suite("fbc_tests.string_.concat")
	fbcu.add_test("concatTest", @concatTest)

end sub

end namespace
