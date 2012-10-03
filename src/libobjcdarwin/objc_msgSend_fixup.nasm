global __darwin_objc_msgSend_fixup

extern objcdarwin_class_lookup
extern objc_msg_lookup
extern sel_get_any_uid
extern objcdarwin_SaveRegisters
extern objcdarwin_RestoreRegisters

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

;__darwin_objc_msgSend_fixed:
;	add rsi, 8
;	jmp __darwin_objc_msgSend WRT ..plt

__darwin_objc_msgSend_fixup:
	; Procedure:
	; 1) get the converted GNU class from an Apple class
	; 2) convert Apple selector to GNU
	; 3) run objc_msg_lookup
	; 4) jump to the pointer returned by objc_msg_lookup
	
	call objcdarwin_SaveRegisters WRT ..plt
	call objcdarwin_class_lookup WRT ..plt
	mov [rsp], rax ; save the converted value
	
	; move the second argument into the first argument
	mov rdi, [rsp+8]
	add rdi, 8 ; the selector itself is the second element of what we receive as SEL
	mov rdi, [rdi]
	call sel_get_any_uid WRT ..plt
	; rax now has the GNU selector
	; move rax to the second argument
	mov rsi, rax

	; restore the first argument
	mov rdi, [rsp]
	call objc_msg_lookup WRT ..plt

	; optimize the next call by fixing the function pointer
	mov rsi, [rsp+8]
	;mov [rsi], rax ; TODO: fixups not working, the target method still isn't getting the selector it expects
	
	call objcdarwin_RestoreRegisters WRT ..plt
	jmp rax

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

__darwin_objc_msgSend_fixup:
	
	mov ecx, [esp+4]
	push ecx ; arg for func call
	
	call objcdarwin_class_lookup ;WRT ..plt
	
	add esp, 4 ; remove argument
	mov [esp+4], eax ; change the class id
	
	mov ecx, [esp+8] ; second argument
	add ecx, 4 ; the selector itself is the second element of what we receive as SEL
	mov ecx, [ecx]
	push ecx
	
	call sel_get_any_uid ;WRT ..plt
	
	add esp, 4
	mov [esp+8], eax
	
	push eax ; reuse the sel_get_any_uid retval
	mov eax, [esp+8]
	push eax ; class id
	
	call objc_msg_lookup ;WRT ..plt
	add esp, 8

	; optimize the next call by fixing the function pointer
	mov ecx, [esp+8]
	;mov [ecx], eax ; TODO: fixups not working, the target method still isn't getting the selector it expects
	
	jmp eax

%else

%error "Unsupported platform"

%endif
