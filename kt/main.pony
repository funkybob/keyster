use "collections"
use "logger"
use "net"

class HandleUDPNotify is UDPNotify
  let _hmap: HashMap[String, String, HashEq[String]] = HashMap[String, String, HashEq[String]].create()

  fun ref not_listening(sock: UDPSocket ref): None val =>
    None

  fun ref received(sock: UDPSocket ref, data: Array[U8] iso, from: NetAddress) =>
    let src = recover val consume data end
    if src.size() == 0 then
      return
    end
    var cmd = String.from_array(src.trim(0, 1))
    var key = try String.from_array(src.trim(1, src.find(0, 0))) else "" end
    var value = String.from_array(src.trim(2 + key.size()))

    var result: String = ""
    match cmd
    | "G" => try result = _hmap(key) end
    | "A" => try _hmap.insert_if_absent(key, value) end
    | "S" => try _hmap.insert(key, value) end
    | "D" => try _hmap.remove(key) end
    else
      result = _hmap.size().string()
    end

    var rsp: Array[U8] iso = recover iso Array[U8].create(2 + key.size() + result.size()) end
    rsp.append(cmd.lower())
    rsp.append(key)
    rsp.push(0)
    rsp.append(result)
    sock.write(consume rsp, from)


actor Main
  new create(env: Env) =>
    let logger = StringLogger(Info, env.out)
    logger.log("Started...\n")
    try
      let socket = UDPSocket(env.root as AmbientAuth, HandleUDPNotify, "", "6347")
    end
