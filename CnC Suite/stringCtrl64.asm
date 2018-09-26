
	extern malloc:proc
	extern free:proc

	.code

; extern "C" STRVECT GetNextLineFromStartIndexW64(int* StartIndex, LPCWSTR buffer);
; ----------------------------------------------------------------------------------------------------------------------------------------------------------------
; Retrieves the string from the startindex of the buffer to the next linefeed/Carriage Return
; Returnes the pointer to the allocated string, which contains the line data
; The 'StartIndex'- Parameter contains the start of the next line after execution or a negative value to indicate an error (see Note section)
;
; Note:
;	This is the unicode version of this function (16bit)
;	Errorcodes: (-1 == no data) (-2 == end of buffer) (-3 == invalid parameter)

	GetNextLineFromStartIndexW64 proc frame

	;function prolog >> --------------------------------------------------------------------------------;

		push rbp									;save callers rbp register
		.pushreg rbp
		push rsi									;save string source
		.pushreg rsi
		push rdi									;save string destination
		.pushreg rdi

		sub rsp, 16									;allocate local stack space
		.allocstack 16

		mov rbp, rsp								;set stackframe pointer
		.setframe rbp, 0

	RBP_RA = 40										;offset from rbp to return address
		.endprolog
	;----------------------------------------------------------------------------------------------------;

	;save arguments to home area >> ---------------------------------------------------------------------;

		mov [rbp + RBP_RA + 8], rcx					;save Startindex pointer
		mov [rbp + RBP_RA + 16], rdx				;save buffer addr
	;----------------------------------------------------------------------------------------------------;

	;check arguments >> ---------------------------------------------------------------------------------;
		
		or rcx, rcx									;test Startindex
		jz no_data									;Startindex was nullptr

		or rdx, rdx									;test buffer
		jz error									;buffer was nullptr

		mov eax, [rcx]								;Startindex-Value to eax
		cdqe										;sign-extend eax to rax
		mov r8, 0									;r8 as comparand
		cmp rax, r8									;check if the StartIndex-parameter is in not negative to avoid an access violation
		jl error
	;----------------------------------------------------------------------------------------------------;

	;calculate start-address >> -------------------------------------------------------------------------;

		;startindex pointer is still in rcx!

		mov rdx, 0									;reset rdx register
		mov edx, [rcx]								;startindex value to edx
		imul rdx, 2									;calculate the offset (*2byte (unicode))

		mov rsi, [rbp + RBP_RA + 16]				;buffer as string instruction source pointer
		add rsi, rdx								;add the offset of the beginning to rsi

		mov [rbp + 8], rsi							;save rsi to the local stack (local-2) for later
	;----------------------------------------------------------------------------------------------------;


	;at first count the characters in the line >> -------------------------------------------------------;

		mov rcx, 0									;reset counter

	@@:	movzx eax, word ptr [rsi + rcx * 2]			;load next char into eax
		inc rcx										;increase char-counter
		or ax, ax									;test for terminating null-character
		jz @F										;end of string found
		cmp ax, 0Ah									;test for linefeed
		jz linef									;end of line found
		cmp ax, 0Dh									;test for carriage return
		jz carry									;carriage return found

		jmp @B										;jump to loop start address

	carry:											;this was a carriage return!
		cmp rcx, 1									;check if this was an empty line
		jz empty_line

		jmp @F

	linef:											;this was a linefeed!
		cmp rcx, 1									;check if this is an empty line
		jz empty_line
	;----------------------------------------------------------------------------------------------------;

	;check for data >> ----------------------------------------------------------------------------------;
	@@:	
		cmp rcx, 1									;compare char-counter with 1 to make sure there is data
		jz no_data									;no data found

		dec rcx										;the last char was the exit trigger and should not be counted
		mov [rbp + RBP_RA + 32], rcx				;save the amount of chars to copy to the home area (r9 - home)

	;if this is the string-end skip the next step >>
		or ax, ax									;test for string-end
		jz stringend								;jump and skip
	;----------------------------------------------------------------------------------------------------;

	;set next line StartIndex >> ------------------------------------------------------------------------;

		mov r8, [rbp + RBP_RA + 8]					;startindex pointer into r8
		movsxd rdx, dword ptr [r8]					;startindex value into edx + sign-extend
		inc rcx										;add one to ignore the line-end-character
		add rdx, rcx								;add the char-counter to rdx
		mov [r8], edx								;set the value of StartIndex to the beginning of the next line

		jmp next

	stringend:										;this is the end of the buffer, so set StartIndex-value to -2

		mov r8, [rbp + RBP_RA + 8]					;startindex pointer into r8
		mov r9, -2									;exitcode to r9
		mov [r8], r9d								;set startindex to exitcode

	next:
		mov rcx, [rbp + RBP_RA + 32]				;recover the amount of chars

	;----------------------------------------------------------------------------------------------------;
	
	;allocate memory for the string >> ------------------------------------------------------------------;
		add rcx, 1									;add one field for the zero terminator
		imul rcx, 2									;calculate the size in bytes (*2 (unicode))

		call malloc									;invoke malloc

		or rax, rax									;test for valid pointer
		jz no_data									;jump to end if malloc failed

		mov [rbp + RBP_RA + 24], rax				;use the third field of the home area to store the pointer to the allocated buffer
	;----------------------------------------------------------------------------------------------------;

	;prepare for string-copy >> -------------------------------------------------------------------------;

		mov rcx, [rbp + RBP_RA + 32]				;recover the amount of character

		mov r9, 0									;use r9 as locator (upcounter)

		mov rdi, rax								;set string destination pointer to the start of the allocated array
		mov rsi, [rbp + 8]							;recover the inital source pointer
	;----------------------------------------------------------------------------------------------------;

	;now copy the string-segment >> ---------------------------------------------------------------------;

	@@:	movzx eax, word ptr [rsi + r9 * 2]			;move the current char to register eax (mem read)
		mov word ptr [rdi + r9 * 2], ax				;move the current char to memory (mem write)
	
		inc r9										;r9 is used to point to the correct memory location
	
		dec rcx										;decrease rcx
		cmp rcx,0									;test the counter for string-end
		jz @F										;jump to string-finalization

		jmp @B										;jump to loop start address

	@@:	mov rax, 0									;select zero terminator
		mov word ptr [rdi + r9 * 2], ax				;set zero terminator

		mov rax, rdi								;set return value to the address of the allocated array

		jmp epilog
	;----------------------------------------------------------------------------------------------------;

	;process the different exit-cases >> ----------------------------------------------------------------;

	no_data:
		mov r8, [rbp + RBP_RA + 8]					;startindex pointer into r8
		mov rdx, -1									;set errorcode
		mov [r8], edx								;set startindex to errorcode		
		mov rax, 0									;set return-value to nullptr

		jmp epilog

	empty_line:
		mov r9, 1									;if rbx is 1 -> it was a carriage return
		mov rdx, 2
		cmp ax, 0Ah
		cmovz r9, rdx								;if rbx is 2 -> it was a linefeed

		mov [rbp + RBP_RA + 32], r9					;save r9 to r9-home

		mov rcx, 10									;5 character * 2byte = 10byte data as parameter for malloc
		call malloc									;allocate mem

		or rax, rax									;update status register with rax
		jz no_data									;return value of malloc was nullptr -> goto error

		mov [rbp + RBP_RA + 24], rax				;save pointer to string

		mov rdi, rax								;set string destination pointer
		mov rax, 5Fh								;rax = underscore
		
		mov word ptr [rdi], ax
		mov word ptr [rdi + 2], ax					;store underscore two times

		mov r9, [rbp + RBP_RA + 32]					;recover r9

		cmp r9, 1									;if rbx is 1 -> it's a carriage return
		jz allo_carry								;-> so jump to ...

		mov rax, 6Ch								;rax = 'l'
		mov word ptr [rdi + 4], ax
		mov rax, 66h								;rax = 'f'
		mov word ptr [rdi + 6], ax
		jmp final
		
	allo_carry:
		mov rax, 63h								;rax = 'c'
		mov word ptr [rdi + 4], ax
		mov rax, 72h								;rax = 'r'
		mov word ptr [rdi + 6], ax

	final:
		mov rax, 0
		mov word ptr [rdi + 8], ax					;set zero terminator

		mov rax, [rbp + RBP_RA + 24]				;set return value to allocated array

		mov r8, [rbp + RBP_RA + 8]					;startindex pointer into r8
		movsxd rcx, dword ptr [r8]					;startindex value into rcx
		inc rcx										;add 1 to the startindex
		mov [r8], ecx								;store it back in StartIndex

		jmp epilog

	error:
		mov r8, [rbp + RBP_RA + 8]					;startindex pointer into r8
		mov r9, -3									;errorcode into r9
		mov [r8], r9d								;StartIndex = -3 (errorvalue)
		mov rax, 0									;return value = nullptr

	;function epilog >> ----------------------------------------------------------------------------;
	epilog:

		add rsp, 16									;clear the local stack space

		pop rdi										;recover caller's rdi
		pop rsi										;recover caller's rsi
		pop rbp										;recover caller's rbp

		ret
	;-----------------------------------------------------------------------------------------------;

	GetNextLineFromStartIndexW64 endp


; extern "C" int GetStringLength64(LPCWSTR buffer);
;----------------------------------------------------------------------------------------------------------------------------------------------------
; returns the amount of characters in buffer including the terminating null character
; ->> negative value indicates an error

	GetStringLengthW64 proc

	;test arguments and prepare for counting >> ------------------------------------------------------;

		or rcx, rcx				;test buffer for nullptr
		jz fail

		mov r11, rcx			;buffer as source in r11
		mov rcx, 0				;rcx is the counter
	;------------------------------------------------------------------------------------------------;

	; count >> --------------------------------------------------------------------------------------;

	@@: movzx eax, word ptr [r11 + rcx * 2]				;charater to eax
		inc rcx											;count the characters
		or ax, ax										;test for nullptr
		jz finalize

		jmp @B
	;------------------------------------------------------------------------------------------------;

	;handle exit conditons >> -----------------------------------------------------------------------;

	fail:
		mov rax, -1
		jmp return
	;------------------------------------------------------------------------------------------------;

	finalize:
		mov rax, rcx
	;------------------------------------------------------------------------------------------------;

	return:
		ret

	GetStringLengthW64 endp


;extern "C" void ReleaseStringVect64(STRVECT buffer);
;------------------------------------------------------------------------------------------------------------------------------------------------------------
;this function releases the strings allocated with malloc

	ReleaseStringVect64 proc frame

	;function prolog >> ----------------------------------------------------------------;

		push rbp				;save caller's stack base pointer
		.pushreg rbp

		sub rsp, 16				;allocate local stack space
		.allocstack 16

		mov rbp, rsp			;save stackpointer
		.setframe rbp, 0

	RBP_RA = 24;
		.endprolog
	;-----------------------------------------------------------------------------------;
		
		call free				;free the memory block

	;function epilog >> ----------------------------------------------------------------;
		
		add rsp, 16				;clear the local stack space
		pop rbp

		ret

	;-----------------------------------------------------------------------------------;

	ReleaseStringVect64 endp

;extern "C" int SetBlockInBufferFromIndexW64(LPWSTR buffer, LPCWSTR appendix, int index);
;-------------------------------------------------------------------------------------------------------------------------------------------------------------
;this function replaces a memory-block in the given buffer, beginning with the index
;this is the unicode version of the function (16bit)

	SetBlockInBufferFromIndexW64 proc
		
		;test arguments >> -------------------------------------------------------------;
			or rcx, rcx			;test buffer for nullptr
			jz fail
			or rdx, rdx			;test appendix for nullptr
			jz fail
			cmp r8, 0			;test index for negative value
			jl fail
		;-------------------------------------------------------------------------------;

		;prepare move operation >> -----------------------------------------------------;
			mov r10, rcx				;buffer as destination in r10
			mov r11, rdx				;appendix as source in r11

			mov rcx, r8
			imul rcx, 2
			add r10, rcx				;add the index-offset to buffer

			mov rcx, 0					;counter to zero
		;-------------------------------------------------------------------------------;
		
		;move >> -----------------------------------------------------------------------;
		@@:	movzx eax, word ptr [r11 + rcx * 2]		;current char to eax (from appendix)

			or ax, ax								;test for terminating null character
			jz finalize

			mov word ptr [r10 + rcx * 2], ax		;char into buffer

			inc rcx
			jmp @B

		;-------------------------------------------------------------------------------;

		fail:
			mov rax, -1
			jmp return
		;-------------------------------------------------------------------------------;
		
		finalize:
			mov rax, rcx
		;-------------------------------------------------------------------------------;

		return:
			ret

	SetBlockInBufferFromIndexW64 endp

;extern "C" int CompareStringsAsmW64(LPCWSTR string1, LPCWSTR string2);
;--------------------------------------------------------------------------------
;This function compares two strings

	CompareStringsAsmW64 proc

		;check arguments >> ---------------------------------------------------;

			or rcx, rcx				;test string1 for nullptr
			jz invalid_arg

			or rdx, rdx				;test string2 for nullptr
			jz invalid_arg
		;----------------------------------------------------------------------;

		;prepare for compare operation >> -------------------------------------;

			mov r8, rcx			;string1 in r8 (start-address of operand 1)
			mov r9, rdx			;string2 in r9 (start-address of operand 2)
			
			mov rcx, 0				;initalize counter
		;----------------------------------------------------------------------;

		;compare >> -----------------------------------------------------------;

		@@:	movzx eax, word ptr [r8 + rcx * 2]			;string1[x] into eax
			movzx edx, word ptr [r9 + rcx * 2]			;string2[x] into edx

			inc rcx

			cmp ax, dx									;compare ...
			jne not_equal

			or ax, ax									;test for zero terminator in ax
			jz check_termination_dx

			or dx, dx									;test for zero terminator in dx, ax was tested before, so if this is zero, the strings are not equal
			jz not_equal

			jmp @B

		;- - - - - - - - - - - - - - - - - - - - - - - 
		check_termination_dx:
			or dx, dx									;ax was zero, if dx is also zero, jump to equal
			jz equal			
			
		;----------------------------------------------------------------------;

		not_equal:
			mov rax, 0
			jmp return

		invalid_arg:
			mov rax, -1
			jmp return

		equal:
			mov rax, 1

		return:
			ret

	CompareStringsAsmW64 endp

;extern "C" int InsertCharacterInBufferW64(LPCWSTR oldBuffer, LPWSTR newBuffer, WCHAR c, int charIndex);
;-------------------------------------------------------------------------------------------------
;this function inserts a wide-char at the specified index

	InsertCharacterInBufferW64 proc

		;check arguments >> --------------------------------------------------------;

			or rcx, rcx					;test oldBuffer for nullptr
			jz fail

			or rdx, rdx					;test newBuffer for nullptr
			jz fail

			mov rax, 0
			cmp r9, rax					;test if the charIndex is negative
			jl fail
		;---------------------------------------------------------------------------;

		;prepare for insert operation >> -------------------------------------------;

			mov r11, rcx				;oldBuffer as source in r11
			mov r10, rdx				;newBuffer as destination in r10

			mov rcx, 0					;reset source counter
			mov rdx, 0					;reset target counter
		;---------------------------------------------------------------------------;

		;process >> ----------------------------------------------------------------;

		@@:	movzx eax, word ptr [r11 + rcx * 2]		;current char to eax
			mov word ptr [r10 + rdx * 2], ax		;current char to target buffer

			inc rcx
			inc rdx
			cmp rcx, r9								;test if the index to insert is reached
			jz insertchar

			cmp ax, 0								;test ax for zero terminator
			jz finalize

			jmp @B


		insertchar:
			mov word ptr [r10 + rdx * 2], r8w		;insertchar to index
			inc rdx

			jmp @B
		;---------------------------------------------------------------------------;
		
		;handle exit conditions >> -------------------------------------------------;

		fail:
			mov rax, -1					;error return value
			jmp return

		finalize:
			mov rax, 1					;success return value

		return:
			ret

	InsertCharacterInBufferW64 endp


;extern "C" int RemoveCharacterFromBufferW64(LPCWSTR oldBuffer, LPWSTR newBuffer, int charIndex);
;---------------------------------------------------------------------------------------------------------------
;this function removes a wide-character at the specified index of oldBuffer and rearranges the data in newBuffer

	RemoveCharacterFromBufferW64 proc

		;check arguments >> -----------------------------------------------------;

			or rcx, rcx					;test oldBuffer for nullptr
			jz fail

			or rdx, rdx					;test newBuffer for nullptr
			jz fail

			cmp r8, 0					;test if charIndex is negative
			jl fail
		;------------------------------------------------------------------------;

		;prepare operation >> ---------------------------------------------------;
			
			mov r11, rcx				;oldBuffer as source in r11
			mov r10, rdx				;newBuffer as target in r10

			mov rcx, 0					;reset source counter
			mov rdx, 0					;reset target counter
		;------------------------------------------------------------------------;

		;process >> -------------------------------------------------------------;
			
		@@:	movzx eax, word ptr [r11 + rcx * 2]			;current char into eax
			mov word ptr [r10 + rdx * 2], ax			;current char to target buffer

			inc rcx
			inc rdx

			cmp rcx, r8									;test if index is reached
			jz discard_char

			cmp ax, 0									;test for zero terminator
			jz finalize

			jmp @B

		discard_char:
			inc rcx
			jmp @B
		;------------------------------------------------------------------------;

		;handle exit conditions >> ----------------------------------------------;
		fail:
			mov rax, -1					;error return value
			jmp return

		finalize:
			mov rax, 1					;success return value

		return:
			ret
		
	RemoveCharacterFromBufferW64 endp


;extern "C" int GetBlockSegmentOutOfBufferW64(LPCWSTR sourceBuffer, LPWSTR targetBuffer, int startIndex, int endIndex);
;------------------------------------------------------------------------------------------------------------------------
;this function retrieves a memory-block out of another memory-block from start- to end-index

	GetBlockSegmentOutOfBufferW64 proc

		;check arguments >> ------------------------------------------------------------;
			
			or rcx, rcx						;test sourceBuffer for nullptr
			jz fail

			or rdx, rdx						;test targetBuffer for nullptr
			jz fail

			cmp r8, 0						;test if startIndex is negative
			jl fail

			cmp r9, 0						;test if endIndex is negative
			jl fail
		;--------------------------------------------------------------------------------;

		;prepare operation >> -----------------------------------------------------------;

			mov r11, rcx					;sourceBuffer as source in r11
			mov r10, rdx					;targetBuffer as destination in r10

			mov rcx, r8						;set source counter
			mov rdx, 0						;set target counter
		;--------------------------------------------------------------------------------;

		;process >> ---------------------------------------------------------------------;

		@@: movzx eax, word ptr [r11 + rcx * 2]			;current char into eax
			mov word ptr [r10 + rdx * 2], ax			;char to target buffer

			inc rcx
			inc rdx

			cmp rcx, r9
			jg finalize

			jmp @B

		;--------------------------------------------------------------------------------;

		;handle exit conditions >> ------------------------------------------------------;

		fail:
			mov rax, -1							;error return value
			jmp return

		finalize:
			mov rax, 0
			mov word ptr [r10 + rdx * 2], ax	;set zero terminator
			mov rax, 1							;success return value

		return:
			ret

	GetBlockSegmentOutOfBufferW64 endp


;extern "C" int InsertBlockInBufferW64(LPWSTR targetBuffer, LPCWSTR oldBuffer, LPCWSTR bufferToInsert, int index);
;------------------------------------------------------------------------------------------------------------------------------
;this function inserts the bufferToInsert in the oldBuffer at the specified index and stores the new string in the targetBuffer

	InsertBlockInBufferW64 proc frame

		;prolog >> ---------------------------------------------------------------------;

			push rbp							;save caller's stackbase
				.pushreg rbp
			push rsi							;save caller's si-source
				.pushreg rsi
			push rdi							;save caller's si-destination
				.pushreg rdi

			sub rsp, 16							;allocate local stack space
				.allocstack 16

			mov rbp, rsp						;set stackframe base pointer
				.setframe rbp, 0

			RBP_RA = 40							;offset from rbp to return address
				.endprolog

		;check arguments >> ------------------------------------------------------------;

			or rcx, rcx							;test targetBuffer for nullptr
			jz fail

			or rdx, rdx							;test oldBuffer for nullptr
			jz fail

			or r8, r8							;test bufferToInsert for nullptr
			jz fail

			cmp r9, 0							;test if index is negative
			jl fail
		;-------------------------------------------------------------------------------;

		;prepare for operation >> ------------------------------------------------------;
			
			mov rdi, rcx						;targetbuffer as destination
			mov rsi, rdx						;oldBuffer as source

			mov rcx, 0							;reset target counter
			mov rdx, 0							;reset source counter

			mov r10, 0							;reset r10 (for use as iterator)
			mov rax, 1

			cmp r9, 0							;if index is 0 -> set r10 flag
			cmovz r10, rax

			cmp r10, 1							;if index is 0 -> set bufferToInsert as source
			jz segment_as_source
		;-------------------------------------------------------------------------------;

		;process >> --------------------------------------------------------------------;

		@@:	movzx eax, word ptr [rsi + rdx * 2]		;current char into eax
			mov word ptr [rdi + rcx * 2], ax		;current char into destination

			inc rcx
			inc rdx

			cmp rcx, r9								;look for matching index
			jz segment_as_source

			cmp ax, 0
			jz check_zero

			jmp @B

		segment_as_source:
			mov r11, rsi						;save oldBuffer source pointer to r11
			mov r12, rdx						;save oldBuffer offset position to r12

			mov rsi, r8							;bufferToInsert is source
			mov rdx, 0							;set offset to zero

			mov r10, 2							;set segment-processed flag

			jmp @B

		check_zero:
			cmp r10, 3
			jz finalize

			cmp r10, 1
			jz fail

		recover_old_source:	
			mov rsi, r11						;recover pointer to oldBuffer
			mov rdx, r12						;recover offset to position in oldBuffer

			mov r10, 3							;set final flag

			dec rcx								;decrease rcx to overwrite the zero terminator

			jmp @B

		;-------------------------------------------------------------------------------;

		;handle exit conditions >> -----------------------------------------------------;

		fail:
			mov rax, -1							;error return code
			jmp return

		finalize:
			mov rax, 1							;success return code

		;function epilog >> ----------------------------------------------------------------------------;

		return:
			add rsp, 16									;clear the local stack space

			pop rdi										;recover caller's rdi
			pop rsi										;recover caller's rsi
			pop rbp										;recover caller's rbp

			ret

	InsertBlockInBufferW64 endp


;extern "C" int RemoveBlockFromBufferW64(LPWSTR targetBuffer, LPCWSTR oldBuffer, int startIndex, int endIndex);
;----------------------------------------------------------------------------------------------------------------
;this function removes a string segment from the given buffer and stores the result in the targetBuffer

	RemoveBlockFromBufferW64 proc frame

		;prolog >> ---------------------------------------------------------------;

			push rbp
				.pushreg rbp
			push rsi
				.pushreg rsi
			push rdi
				.pushreg rdi

			sub rsp, 16						;allocate local stack space
				.allocstack 16

			mov rbp, rsp					;set stackframe base
				.setframe rbp, 0

			RBP_RA = 40						;offset from rbp to return address
				.endprolog

		;check arguments >> ------------------------------------------------------;

			or rcx, rcx						;test targetBuffer for nullptr
			jz fail

			or rdx, rdx						;test oldBuffer for nullptr
			jz fail

			cmp r8, 0						;test if startIndex is negative
			jl fail

			cmp r9, 0						;test if endIndex is negative
			jl fail
		;-------------------------------------------------------------------------;

		;prepare operation >> ----------------------------------------------------;

			mov rsi, rdx					;oldBuffer as source
			mov rdi, rcx					;targetBuffer as destination

			mov rcx, 0						;reset source counter
			mov rdx, 0						;reset destination counter

			cmp r8, 0						;test if startIndex == 0
			jz skip_segment
		;-------------------------------------------------------------------------;

		;process >> --------------------------------------------------------------;

		@@:	mov ax, word ptr [rsi + rcx * 2]	;source char to ax
			mov word ptr [rdi + rdx * 2], ax	;char to destination

			inc rcx
			inc rdx

			cmp rcx, r8							;test for startIndex
			jz skip_segment

			cmp ax, 0							;test for string-end
			jz finalize

			jmp @B

		skip_segment:
			mov r10, r9							;endIndex to r10
			sub r10, r8							;subtract startIndex to get the scope

			add rcx, r10						;add the scope to the counter
			inc rcx								;endindex must be removed too!

			jmp @B
		;-------------------------------------------------------------------------;

		;handle exit conditions >> -----------------------------------------------;

		fail:
			mov rax, -1
			jmp return

		finalize:
			mov rax, 1

		;epilog >> ---------------------------------------------------------------;

		return:
			add rsp, 16							;clear the local stack space

			pop rdi
			pop rsi
			pop rbp

			ret

	RemoveBlockFromBufferW64 endp

end

