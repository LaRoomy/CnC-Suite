
	.model flat, c

	extern malloc:proc
	extern free:proc

	.code

; extern "C" STRVECT GetNextLineFromStartIndexW32(int* StartIndex, LPCWSTR buffer);
; -------------------------------------------------------------------------------------------
; Retrieves the string from the startindex of the buffer to the next linefeed/Carriage Return
; Returnes the pointer to the allocated string, which contains the line data
; The 'StartIndex'- Parameter contains the start of the next line after execution
; Note: This is the unicode version of this function (16bit)
; Errorcodes: (-1 == no data) (-2 == end of buffer) (-3 == invalid parameter)

	GetNextLineFromStartIndexW32 proc

	;function prolog >>
		push ebp									;save stack-base-pointer
		mov ebp, esp								;load the stack pointer into base
		push ebx									;save ebx
		push ecx									;save ecx
		push edx									;save edx
		push esi									;save esi
		push edi									;save edi

		mov ebx, [ebp + 8]							;load StartIndex-Pointer in ebx
		mov esi, [ebp + 12]							;load buffer to string-instruction-source-pointer register

	;check formal parameter
		or ebx, ebx									;test StartIndex
		jz no_data									;StartIndex was nullptr -> jump to end
		or esi, esi									;test buffer
		jz error									;buffer is nullptr -> jump to end

		mov ecx, [ebx]
		mov edx, 0
		cmp ecx, edx								;check if the StartIndex-parameter is in not negative to avoid an access violation
		jl error

	;count the characters and check the startindex for overrun condition >> (disabled for speed optimization -> do that before calling the function)
		;push esi
		;call GetStringLengthW
		;pop esi
		;cmp eax, [ebx]
		;jle error

	;calculate start-address
		mov edx, [ebx]								;load the value of StartIndex in edx
		imul edx, 2									;calculate the offset (*2byte (unicode))

		add esi, edx								;add the offset to the instruction source pointer
		push esi									;save esi for second run
		mov ecx, 0									;reset char-counter

	;at first count the characters in the line >>
	@@:	lodsw										;load next char into ax
		inc ecx										;increase char-counter
		or ax, ax									;test for terminating null-character
		jz @F										;end of string found
		cmp ax, 0Ah									;test for linefeed
		jz linef									;end of line found
		cmp ax, 0Dh									;test for carriage return
		jz carry									;carriage return found

		jmp @B										;jump to loop start address

	carry:											;this was a carriage return escape!
		cmp ecx, 1									;check if this was an empty line
		jz empty_line

		jmp @F

	linef:											;this was a linefeed escape!
		cmp ecx, 1									;check if this is an empty line
		jz empty_line


	;check for data >>
	@@:	
		cmp ecx, 1									;compare char-counter with 1 to make sure there is data
		jz no_data									;no data found

		push ecx									;save the amount of chars to copy

	;if this is the string-end skip the next step >>
		or ax, ax									;test for string-end
		jz stringend								;jump and skip


	;set next line StartIndex >>		
		mov ebx, [ebp + 8]							;address of StartIndex into ebx
		mov edx, [ebx]								;value of StartIndex into edx
		add edx, ecx								;add the counter to StartIndex
		mov [ebx], edx								;set the value of StartIndex to the beginning of the next line

		jmp next

	stringend:										;this is the end of the buffer, so set StartIndex-value to -2
		mov ebx, [ebp + 8]
		mov edx, -2
		mov [ebx], edx

	next:
		pop ecx										;recover the amount of chars
	
	;allocate memory for the string >>
		imul ecx, 2									;calculate the size in bytes (*2 (unicode))
		push ecx									;push the formal parameter for the malloc function onto the stack
		call malloc									;invoke malloc

		pop ecx										;pop ecx to make sure the stackpointer points to the right address
													;and we can use the register for down-counting
		
		or eax, eax									;test for valid pointer
		jz no_data									;jump to end if malloc failed

	;prepare for string-copy
		push eax									;save pointer to allocated buffer

		mov eax, ecx								;recalculate the amount of chars
		mov ecx, 2									;""
		mov edx, 0									;""
		idiv ecx									;""
		mov ecx, eax								;""
		dec ecx										;subtract the terminating null-character

		pop eax										;recover the address of the allocated buffer
		mov edi, eax								;set string destination pointer to the start of the allocated array
		pop esi										;recover the inital source pointer

	;now copy the string-segment >>
	@@:	movsw										;move data from source(esi) to destination(edi) string-ptr
		dec ecx										;decrease ecx
		cmp ecx,0									;test the counter for string-end
		jz @F										;jump to string-finalization		
		jmp @B										;jump to loop start address

	@@:	push eax
		mov eax, 0									;select zero terminator
		stosw										;set zero terminator

		pop eax										;set return value

		jmp epilog

	no_data:
		mov ebx, [ebp + 8]							;get Startindex-Pointer
		mov edx, -1									;set errorcode
		mov [ebx], edx								;set StartIndex to errorcode	
		mov eax, 0									;set return-value to nullptr
		add esp, 4									;set the stack-pointer to the correct position (discard the 'push esi' instruction - line 49)
		jmp epilog

	empty_line:
		mov ebx, 1									;if ebx is 1 -> it was a carriage return
		mov edx, 2
		cmp ax, 0Ah
		cmovz ebx, edx								;if ebx is 2 -> it was a linefeed

		push 10										;5 character * 2byte = 10byte data as parameter for malloc
		call malloc									;allocate mem
		add esp, 4									;remove parameter from stack

		or eax, eax									;update status register with eax
		jz no_data									;return value of malloc was nullptr -> goto error

		push eax									;save pointer to string
		mov edi, eax								;set string destination pointer
		mov eax, 5Fh								; eax = underscore
		stosw
		stosw										;store underscore two times

		cmp ebx, 1									;if ebx is 1 -> it's a carriage return
		jz allo_carry								;-> so jump to ...

		mov eax, 6Ch								;eax = 'l'
		stosw
		mov eax, 66h								;eax = 'f'
		stosw
		jmp final
		
	allo_carry:
		mov eax, 63h								;eax = 'c'
		stosw
		mov eax, 72h								;eax = 'r'
		stosw

	final:
		mov eax, 0
		stosw										;set zero terminator

		mov eax, [ebp + 8]							;address of StartIndex to eax
		mov ebx, [eax]								;value of StartIndex to ebx
		inc ebx										;add 1 to the index
		mov [eax], ebx								;store it back in StartIndex

		pop eax										;recover pointer to string as return value
		add esp, 4									;set the stack-pointer to the correct position (discard the 'push esi' instruction - line 49)
		jmp epilog

	error:
		mov eax, [ebp + 8]
		mov ebx, -3
		mov [eax], ebx								;StartIndex = -3 (errorvalue)
		mov eax, 0									;return value = nullptr

	;function epilog >>
	epilog:
		pop edi										;recover edi
		pop esi										;recover esi
		pop edx										;recover edx
		pop ecx										;recover ecx
		pop ebx										;recover ebx
		pop ebp										;recover old stack-base

		ret

	GetNextLineFromStartIndexW32 endp


; extern "C" int GetStringLength32(LPCWSTR buffer);
;-------------------------------------------------------------------------------------
; returns the amount of characters in buffer including the terminating null character
; ->> negative value indicates an error

	GetStringLengthW32 proc

	;function prolog >>
		push ebp
		mov ebp, esp
		push esi
		push ecx

		mov esi, [ebp + 8]			;address of buffer to esi
		or esi, esi					;update status register
		jz error_no_buffer			;buffer was nullptr

		mov ecx, 0

	@@:	lodsw						;wchar_t to ax
		inc ecx						;count with ecx
		or ax, ax					;test for terminating null character (update status register)
		jz success

		jmp @B
		

	error_no_buffer:
		mov eax, -1
		jmp f_epilog

	success:
		mov eax, ecx				;set the counter as return value

	;function epilog >>
	f_epilog:
		pop ecx
		pop esi
		pop ebp

		ret

	GetStringLengthW32 endp


;extern "C" void ReleaseStringVect32(STRVECT buffer);
;---------------------------------------------------------
;this function releases the strings allocated with malloc

	ReleaseStringVect32 proc

		push ebp
		mov ebp, esp
		
		push [ebp + 8]				;set parameter for the free-function (pointer to memory block)
		call free					;free the memory block
		add esp, 4					;remove the parameter from the stack

		pop ebp

		ret

	ReleaseStringVect32 endp

;extern "C" int SetBlockInBufferFromIndexW32(LPWSTR buffer, LPCWSTR appendix, int index);
;----------------------------------------------------------------------------------------
;this function replaces a memory-block in the given buffer, beginning with the index
;this is the unicode version of the function (16bit)

	SetBlockInBufferFromIndexW32 proc
		
		;function prolog >>

			push ebp
			mov ebp, esp
			push ecx
			push edx
			push esi
			push edi

		;check the given parameter >>

			mov eax, [ebp + 8]				;buffer
			or eax, eax
			jz error_invalid_parameter		;buffer is nullptr

			mov eax, [ebp + 12]				;appendix
			or eax, eax
			jz error_invalid_parameter		;appendix is nullptr

			mov eax, [ebp + 16]				;index
			cmp eax, 0
			jl error_invalid_parameter		;index is negative

		;prepare parameter for loop operation >>
			
			mov edi, [ebp + 8]				;buffer start-address to destination
			mov esi, [ebp + 12]				;appendix start-address to source

			mov edx, [ebp + 16]				;index to edx
			imul edx, 2						;multiplicate with 2 (unicode) -> result in edx

			add edi, edx					;set destination copy processing start address

			mov edx, 0
			mov ecx, 0

		;start char-copy >>
		;-- loop -------------------------------------------	

			@@:		movsw
					inc ecx
					cmp [esi], dx
					jz complete

					jmp @B

		;-- loop end ---------------------------------------

		;finalization >>

			error_invalid_parameter:
				mov eax, -1					;set error return value
				jmp epilog

			complete:
				mov eax, ecx
				jmp epilog

		;function epilog >>

			epilog:
				pop edi
				pop esi
				pop edx
				pop ecx
				pop ebp

				ret

	SetBlockInBufferFromIndexW32 endp

;extern "C" int CompareStringsAsmW32(LPCWSTR string1, LPCWSTR string2);
;--------------------------------------------------------------------------------
;This function compares two strings

	CompareStringsAsmW32 proc

		;function prolog >>

			push ebp
			mov ebp, esp
			push esi
			push edi

		;set parameter >>
			
			mov esi, [ebp + 8]				;string1 in esi
			or esi, esi
			jz error_invalid_parameter		;string1 is nullptr

			mov edi, [ebp + 12]				;string2 in edi
			or edi, edi
			jz error_invalid_parameter		;string2 is nullptr

			mov eax, 0
;>>--------------------------------------------------------------------------------loop------->>-------------------------------------------------
	@@:		cmpsw
			jne epilog						;character not equal -> jump to end -> return value eax is already 0

			cmp [esi], ax
			jz check_edi					;end of string1 is reached -> look if string2 is also over

			cmp [edi], ax
			jz epilog						;end of string2 is reached -> string1 was previously checked and was not over -> strings are not equal!

			jmp @B
;<<------------------------------------------------------------------------------------------<<--------------------------------------------------

			check_edi:
							cmp [edi], ax
							jz equal		;string2 end is also reached -> set equal return value

							jmp epilog		;string2 end is not reached -> strings not equal -> return value 0 is already set in eax
					
			equal:
							mov eax, 1		;set equal return value
							jmp epilog		;goto end
				

			error_invalid_parameter:
						mov eax, -1			;set error return value
						jmp epilog			;goto end

		;function epilog >>

			epilog:
				pop edi
				pop esi
				pop ebp

				ret

	CompareStringsAsmW32 endp

;extern "C" int InsertCharacterInBufferW32(LPCWSTR oldBuffer, LPWSTR newBuffer, WCHAR c, int charIndex);
;-------------------------------------------------------------------------------------------------
;this function inserts a wide-char at the specified index

	InsertCharacterInBufferW32 proc

	;prolog >>
		
		push ebp
		mov ebp, esp
		push esi
		push edi
		push ebx
		push ecx
		push edx

	;check values and prepare copy-operation >>

		mov esi, [ebp + 8]			;oldBuffer as source
		or esi, esi
		jz fail						;error - oldBuffer is nullptr

		mov edi, [ebp + 12]			;newBuffer as destination
		or edi, edi
		jz fail						;error - newBuffer is nullptr

		mov ebx, 0					;set comparand for exit condition

		mov edx, [ebp + 20]			;index in edx
		mov ecx, 0					;counter reset

		cmp edx, 0
		jz index_reached			;if the char must be inserted at the first position jump direct to setter part

;-------------------------------------------------------------------------------- loop --------
	@@: movsw
		inc ecx
		cmp ecx, edx
		jz index_reached

		cmp [esi], bx				;look for exit conditon (zero terminator)
		jz finalize

		jmp @B
;----------------------------------------------------------------------------------------------
	index_reached:		
		mov ax, [ebp + 16]			;c in ax
		stosw						;c at index position
		jmp @B						;back to loop

	finalize:
		mov ax, 0
		stosw						;set zero terminator
		mov eax, 1					;set success return value
		jmp epilog

	fail:
		mov eax, -1					;-1 == error_invalid_parameter
		jmp epilog

	epilog:
		pop edx
		pop ecx
		pop ebx
		pop edi
		pop esi
		pop ebp

		ret

	InsertCharacterInBufferW32 endp


;extern "C" int RemoveCharacterFromBufferW32(LPCWSTR oldBuffer, LPWSTR newBuffer, int charIndex);
;---------------------------------------------------------------------------------------------------------------
;this function removes a wide-character at the specified index of oldBuffer and rearranges the data in newBuffer

	RemoveCharacterFromBufferW32 proc
		
		;prolog >>

			push ebp
			mov ebp, esp
			push esi
			push edi
			push ebx
			push ecx
			push edx


		;verify parameter and prepare sort operation >>

			mov esi, [ebp + 8]			;oldBuffer as source
			or esi, esi
			jz fail						;oldBuffer is nullptr

			mov edi, [ebp + 12]			;newBuffer as destination
			or esi, esi
			jz fail						;newBuffer is nullptr

			mov edx, [ebp + 16]			;charIndex in edx
			cmp edx, 0
			jl fail						;charIndex is negative

			mov ebx, 0					;set comparant for exit-condition

			mov ecx, 0					;reset counter
			cmp edx, ecx
			je discard					;if char-index is zero jump direct to discard segment

;--------------------------------------------------------------------------- loop --------------
		@@: movsw
			inc ecx
			cmp ecx, edx				;look for the desired index
			je discard

			cmp [esi], bx				;check for zero-terminator
			je finalize

			jmp @B
;-----------------------------------------------------------------------------------------------
		discard:
			add esi, 2
			cmp [esi], bx
			je finalize

			jmp @B

		finalize:
			mov ax, 0
			stosw						;set zero-terminator on newBuffer
			mov eax, 1					;set success return value
			jmp epilog

		fail:
			mov eax, -1					;set error return value
			jmp epilog

		
		epilog:
			pop edx
			pop ecx
			pop ebx
			pop edi
			pop esi
			pop ebp

		ret

	RemoveCharacterFromBufferW32 endp


;extern "C" int GetBlockSegmentOutOfBufferW32(LPCWSTR sourceBuffer, LPWSTR targetBuffer, int startIndex, int endIndex);
;------------------------------------------------------------------------------------------------------------------------
;this function retrieves a memory-block out of another memory-block from start- to end-index

	GetBlockSegmentOutOfBufferW32 proc

		;prolog >>

			push ebp
			mov ebp, esp

			push esi
			push edi
			push ebx
			push ecx
			push edx

		;validate values >>

			mov esi, [ebp + 8]				;sourceBuffer as source
			or esi, esi
			jz fail							;sourceBuffer is nullptr (error)

			mov edi, [ebp + 12]				;targetBuffer as destination
			or edi, edi
			jz fail							;targetBuffer is nullptr (error)

			mov ebx, [ebp + 16]				;startIndex in ebx
			cmp ebx, 0
			jl fail							;startIndex is below zero (error)

			mov ebx, [ebp + 20]				;endIndex in ebx
			cmp ebx, 0
			jl fail							;endIndex is below zero (error)

		;prepare parameter for loop >>

			mov edx, [ebp + 16]				;startIndex in edx
			imul edx, 2						;multiplicate with 2 (unicode)
			add esi, edx					;add result (in byte) to the source-pointer

			mov ecx, [ebp + 16]				;startIndex in ecx (as counter)
			inc ebx							;set the exit-condition (counter == (endIndex + 1))

;---------------------------------------------------------------------------------------- loop -------
		@@:	movsw
			inc ecx
			cmp ecx, ebx
			jz finalize

			jmp @B
;-----------------------------------------------------------------------------------------------------

		finalize:
			mov ax, 0
			stosw							;set the terminating null character
			mov eax, 1						;set the success return code
			jmp epilog

		fail:
			mov eax, -1						;set the error return code
			jmp epilog

		epilog:
			pop edx
			pop ecx
			pop ebx
			pop edi
			pop esi		
			pop ebp

		ret

	GetBlockSegmentOutOfBufferW32 endp


;extern "C" int InsertBlockInBufferW32(LPWSTR targetBuffer, LPCWSTR oldBuffer, LPCWSTR bufferToInsert, int index);
;------------------------------------------------------------------------------------------------------------------------------
;this function inserts the bufferToInsert in the oldBuffer at the specified index and stores the new string in the targetBuffer

	InsertBlockInBufferW32 proc

		;prolog >>

			push ebp
			mov ebp, esp

			push ebx
			push ecx
			push edx
			push esi
			push edi

			;check parameter >>

				mov edi, [ebp + 8]			;targetBuffer as destination
				or edi, edi
				jz invalid_parameter		;targetBuffer is nullptr

				mov esi,[ebp + 12]			;oldBuffer as source
				or esi, esi
				jz invalid_parameter		;oldBuffer is nullptr

				mov ebx, [ebp + 16]			;check bufferToInsert
				or ebx, ebx
				jz invalid_parameter		;bufferToInsert is nullptr

				mov edx, [ebp + 20]			;index in edx as comparant
				cmp edx, 0
				jl invalid_parameter		;index is negative

				;prepare loop >>
					
					mov ebx, 1				;exit trigger

					mov ecx, 0
					cmp ecx, edx
					jz segment_as_source					

;-----------------------------------------------------------------------------------------------> loop --------------------
				@@:	movsw

					inc ecx
					cmp ecx, edx
					jz segment_as_source

					mov ax, 0
					cmp [esi], ax			;check for zero-terminator at the source location
					jz check_final

					jmp @B

				segment_as_source:
					push esi				;save the old source-position of the oldBuffer
					mov esi, [ebp + 16]		;bufferToInsert as new source

					jmp @B

				check_final:
					cmp ebx, 0
					jz finalize

					pop esi
					mov ax, 0
					cmp [esi], ax
					jz finalize

					mov ebx, 0				;set exit condition on next execution of 'check_final'					
					jmp @B

;--------------------------------------------------------------------------------------------------------------------------

		finalize:
			mov ax, 0
			stosw							;set zero-terminator on targetBuffer

			mov eax, 1						;set success return value
			jmp epilog

		invalid_parameter:
			mov eax, -1						;set error return value
			jmp epilog

		epilog:
			pop edi
			pop esi
			pop edx
			pop ecx
			pop ebx
			pop ebp

		ret

	InsertBlockInBufferW32 endp


;extern "C" int RemoveBlockFromBufferW32(LPWSTR targetBuffer, LPCWSTR oldBuffer, int startIndex, int endIndex);
;----------------------------------------------------------------------------------------------------------------
;this function removes a string segment from the given buffer and stores the result in the targetBuffer

	RemoveBlockFromBufferW32 proc

		;prolog >>

			push ebp
			mov ebp, esp

			push esi
			push edi
			push ebx
			push ecx
			push edx

			;check parameter >>

				mov edi, [ebp + 8]			;targetBuffer as destination
				or edi, edi
				jz invalid_parameter

				mov esi, [ebp + 12]			;oldBuffer as source
				or esi, esi
				jz invalid_parameter
				
				mov ebx, [ebp + 16]			;startIndex in ebx
				cmp ebx, 0
				jl invalid_parameter

				mov edx, [ebp + 20]			;endIndex in edx
				cmp edx, 0
				jl invalid_parameter

			;prepare for loop >>
				
				inc edx

				mov ecx, 0					;reset counter register
				cmp ecx, ebx				;look if the segment is at the beginning of the buffer
				jz skip_segment

;--------------------------------------------------------------------------> loop <-----------------------------
			@@: movsw

				inc ecx		
				cmp ecx, ebx				;look for startIndex
				jz skip_segment

				mov ax, 0
				cmp [esi], ax				;look for zero-terminator
				jz finalize

				jmp @B

			skip_segment:					
				sub edx, ebx				;calculate the offset to endIndex
				imul edx, 2					
				add esi, edx				;add the offset to the source-pointer

				mov ax, 0
				cmp [esi], ax				;check for zero-terminator
				jz finalize					;if this is the stringend -> go direct to finalization

				jmp @B
;------------------------------------------------------------------------------------------------------------------

		finalize:
			
			mov ax, 0
			stosw							;set zero terminator

			mov eax, 1						;set success return code
			jmp epilog

		invalid_parameter:
			mov eax, -1						;set error return code
			jmp epilog

		epilog:
			pop edx
			pop ecx
			pop ebx
			pop edi
			pop esi
			pop ebp

		ret

	RemoveBlockFromBufferW32 endp

end

