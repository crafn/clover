#error Documentation only

/// 'global' namespace should contain things that most subsystems need and which
/// don't have better place to go.
///
/// Pulling from other namespaces to 'global' namespace with 'using'
/// directive is preferred:
/// When there's temptation to add something to 'global' namespace, think hard
/// if it is _strictly_ project specific and if it could be put in 'util' instead.
///
/// If an utility class uses some global manager it probably is project specific.
