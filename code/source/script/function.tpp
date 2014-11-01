template <typename R, typename... Args>
FunctionEntry<R(Args...)>::FunctionEntry(asIScriptFunction& func, Args... args):
	arguments(args...),
	function(&func){
	function->AddRef();
}

template <typename R, typename... Args>
FunctionEntry<R(Args...)>::FunctionEntry(FunctionEntry&& other):
	arguments(std::move(other.arguments)),
	function(other.function){
	other.function= nullptr;
}

template <typename R, typename... Args>
FunctionEntry<R(Args...)>::~FunctionEntry(){
	if (function)
		function->Release();
}

template <typename R, typename... Args>
FunctionEntry<R(Args...)>& FunctionEntry<R(Args...)>::operator=(FunctionEntry&& other){
	arguments= std::move(other.arguments);
	function= other.function;
	
	other.function= nullptr;
	
	return *this;
}


template <typename R, typename... Args>
Function<R(Args...)>::Function()
	: function(nullptr){
}

template <typename R, typename... Args>
Function<R(Args...)>::Function(asIScriptFunction& func):
	function(&func){
	
	function->AddRef();
}

template <typename R, typename... Args>
Function<R(Args...)>::Function(const Function& other):
	function(other.function){

	ensure(function);
	function->AddRef();
}

template <typename R, typename... Args>
Function<R(Args...)>::Function(Function&& other):
	function(other.function){
	other.function= nullptr;
}

template <typename R, typename... Args>
Function<R(Args...)>::~Function(){
	if (function)
		function->Release();
}

template <typename R, typename... Args>
Function<R(Args...)>& Function<R(Args...)>::operator=(const Function& other){
	if (&other != this){
		if (function)
			function->Release();
			
		function= other.function;
		ensure(function);
		function->AddRef();
	}
	
	return *this;
}

template <typename R, typename... Args>
Function<R(Args...)>& Function<R(Args...)>::operator=(Function&& other){
	if (&other != this){
		if (function)
			function->Release();
			
		function= other.function;
		other.function= nullptr;
	}
	
	return *this;
}

template <typename R, typename... Args>
FunctionEntry<R(Args...)> Function<R(Args...)>::operator()(Args... args) const {
	ensure(function);
	return (FunctionEntry<R(Args...)>(*function, args...));
}
