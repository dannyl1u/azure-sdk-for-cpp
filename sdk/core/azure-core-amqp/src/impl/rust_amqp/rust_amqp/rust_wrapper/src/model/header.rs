// Copyright (c) Microsoft Corporation. All Rights Reserved.
// Licensed under the MIT License.

// cspell: words amqp amqpvalue repr

use super::value::RustAmqpValue;
use crate::call_context::{call_context_from_ptr_mut, RustCallContext};
use azure_core_amqp::{
    messaging::AmqpMessageHeader,
    value::{AmqpComposite, AmqpDescriptor, AmqpValue},
};
use std::mem;

pub struct RustMessageHeader {
    inner: AmqpMessageHeader,
}

impl RustMessageHeader {
    pub(crate) fn new(header: AmqpMessageHeader) -> Self {
        Self { inner: header }
    }

    pub(crate) fn get(&self) -> &AmqpMessageHeader {
        &self.inner
    }
}

#[no_mangle]
extern "C" fn header_create() -> *mut RustMessageHeader {
    Box::into_raw(Box::new(RustMessageHeader {
        inner: AmqpMessageHeader::default(),
    }))
}

#[no_mangle]
unsafe extern "C" fn header_destroy(header: *mut RustMessageHeader) {
    mem::drop(Box::from_raw(header));
}

#[no_mangle]
unsafe extern "C" fn header_get_durable(
    header: *const RustMessageHeader,
    durable: &mut bool,
) -> i32 {
    let header = { &*header };
    *durable = header.inner.durable;
    0
}

#[no_mangle]
unsafe extern "C" fn header_get_priority(
    header: *const RustMessageHeader,
    priority: &mut u8,
) -> i32 {
    let header = { &*header };
    *priority = header.inner.priority;
    0
}

#[no_mangle]
unsafe extern "C" fn header_get_ttl(
    header: *const RustMessageHeader,
    time_to_live: &mut u64,
) -> i32 {
    let header = { &*header };
    if let Some(ttl) = header.inner.time_to_live {
        *time_to_live = ttl.as_millis() as u64;
        0
    } else {
        *time_to_live = 0;
        1
    }
}

#[no_mangle]
unsafe extern "C" fn header_get_first_acquirer(
    header: *const RustMessageHeader,
    first_acquirer: &mut bool,
) -> i32 {
    let header = { &*header };

    *first_acquirer = header.inner.first_acquirer;
    0
}

#[no_mangle]
unsafe extern "C" fn header_get_delivery_count(
    header: *const RustMessageHeader,
    delivery_count: &mut u32,
) -> i32 {
    let header = { &*header };
    *delivery_count = header.inner.delivery_count;
    0
}

#[no_mangle]
unsafe extern "C" fn header_set_durable(
    call_context: *mut RustCallContext,
    options: *mut RustMessageHeader,
    durable: bool,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    options.inner.durable = durable;
    0
}

#[no_mangle]
unsafe extern "C" fn header_set_priority(
    call_context: *mut RustCallContext,
    options: *mut RustMessageHeader,
    priority: u8,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    options.inner.priority = priority;
    0
}

#[no_mangle]
unsafe extern "C" fn header_set_ttl(
    call_context: *mut RustCallContext,
    options: *mut RustMessageHeader,
    time_to_live: u64,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    options.inner.time_to_live = Some(std::time::Duration::from_millis(time_to_live));
    0
}

#[no_mangle]
unsafe extern "C" fn header_set_first_acquirer(
    call_context: *mut RustCallContext,
    options: *mut RustMessageHeader,
    first_acquirer: bool,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    options.inner.first_acquirer = first_acquirer;
    0
}

#[no_mangle]
unsafe extern "C" fn header_set_delivery_count(
    call_context: *mut RustCallContext,
    options: *mut RustMessageHeader,
    delivery_count: u32,
) -> i32 {
    let _call_context = call_context_from_ptr_mut(call_context);
    let options = &mut *options;
    options.inner.delivery_count = delivery_count;
    0
}

#[no_mangle]
unsafe extern "C" fn amqpvalue_get_header(
    value: *const RustAmqpValue,
    header: &mut *mut RustMessageHeader,
) -> i32 {
    let value = &*value;
    match &value.inner {
        AmqpValue::Described(value) => match value.descriptor() {
            AmqpDescriptor::Code(0x70) => {
                let h = value.value();
                match h {
                    AmqpValue::List(c) => {
                        *header = Box::into_raw(Box::new(RustMessageHeader {
                            inner: AmqpMessageHeader::from(c.clone()),
                        }));
                        0
                    }
                    _ => {
                        *header = std::ptr::null_mut();
                        println!("Invalid header type: {:?}", h);
                        1
                    }
                }
            }
            _ => {
                *header = std::ptr::null_mut();
                1
            }
        },
        _ => 1,
    }
}

#[no_mangle]
unsafe extern "C" fn amqpvalue_create_header(
    header: *const RustMessageHeader,
) -> *mut RustAmqpValue {
    let header = &*header;
    let value = AmqpValue::Composite(Box::new(AmqpComposite::new(
        AmqpDescriptor::Code(0x70),
        header.inner.clone(),
    )));
    Box::into_raw(Box::new(RustAmqpValue { inner: value }))
}

#[cfg(test)]
mod tests {

    use azure_core_amqp::Deserializable;

    use crate::model::value::{amqpvalue_encode, amqpvalue_get_encoded_size};

    use super::*;

    #[test]
    fn test_amqpvalue_create_header() {
        unsafe {
            let header = RustMessageHeader {
                inner: AmqpMessageHeader {
                    priority: 3,
                    ..Default::default()
                },
            };
            let value = amqpvalue_create_header(&header);

            let mut size: usize = 0;
            assert_eq!(
                { amqpvalue_get_encoded_size(value, &mut size as *mut usize) },
                0
            );
            let mut buffer: Vec<u8> = vec![0; size];
            assert_eq!({ amqpvalue_encode(value, buffer.as_mut_ptr(), size) }, 0);

            let deserialized = <AmqpValue as Deserializable<AmqpValue>>::decode(buffer.as_slice());
            assert!(deserialized.is_ok());
            let deserialized = deserialized.unwrap();
            println!("Deserialized: {:?}", deserialized);
        }
    }
}
